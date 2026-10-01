# runs a function of the game's executable and the function of source/ that
# was decompiled from it, with the same arguments, in a processor emulator
# (Unicorn), and compares what they return and what they write
#
# Nothing of the game or of source/ runs on the computer itself: the
# executable (disc/default.xbe) and source/, compiled for 32-bit x86 and
# linked at an address of its own, are loaded into the emulator's memory.
#
# symbols/functions.csv gives, for each function decompiled, its address in
# the executable and where the executable's function takes each argument:
# a register, or "stack".

import csv
import os
import shutil
import struct
import subprocess
from dataclasses import dataclass, field
from pathlib import Path

from unicorn import UC_ARCH_X86, UC_HOOK_CODE, UC_MODE_32, Uc, UcError
from unicorn import x86_const as x86

ROOT = Path(__file__).resolve().parent.parent
XBE_PATH = ROOT / "disc" / "default.xbe"
SYMBOLS_PATH = ROOT / "symbols" / "functions.csv"
BUILD = ROOT / "build" / "difftest"

# the emulator's memory
IMAGE_ADDRESS = 0x20000000  # source/, compiled
SCRATCH_ADDRESS = 0x60000000  # the buffers of the arguments
SCRATCH_SIZE = 0x01000000
STACK_ADDRESS = 0x70000000
STACK_SIZE = 0x00100000
RETURN_ADDRESS = 0x7FFF0000  # a call returns here, where the emulator stops
KERNEL_ADDRESS = 0x7FFE0000  # the functions of the console's kernel, which the emulator does not have
KERNEL_SIZE = 0x4000

KERNEL_THUNK_KEYS = (0x5B6D40B6, 0xEFB1F152)
MAXIMUM_INSTRUCTIONS = 20_000_000

REGISTERS = {
    "eax": x86.UC_X86_REG_EAX,
    "ebx": x86.UC_X86_REG_EBX,
    "ecx": x86.UC_X86_REG_ECX,
    "edx": x86.UC_X86_REG_EDX,
    "esi": x86.UC_X86_REG_ESI,
    "edi": x86.UC_X86_REG_EDI,
}

COMPILE_FLAGS = [
    "--target=i686-unknown-linux-gnu",
    "-ffreestanding",
    "-fno-pic",
    "-fno-stack-protector",
    "-fno-asynchronous-unwind-tables",
    # the reals in the SSE registers, in single precision, as most of the
    # game's code has them; the console's processor has SSE and not SSE2
    "-msse",
    "-mno-sse2",
    "-mfpmath=sse",
    "-fno-math-errno",
    # a product and a sum stay two operations, each rounded
    "-ffp-contract=off",
    "-O2",
    "-std=c11",
    "-Wall",
    "-Wno-four-char-constants",
    "-Wno-multichar",
    f"-I{ROOT / 'source'}",
    f"-I{ROOT / 'tools' / 'difftest_libc' / 'include'}",
]


class DifftestError(Exception):
    pass


def kernel_query_system_time(mu: Uc, time_address: int) -> int:
    """KeQuerySystemTime: a fixed time, so that every run is the same."""
    mu.mem_write(time_address, struct.pack("<Q", 0x01C4A504_93122C35))
    return 0


# the functions of the kernel that the tests have, by their number: the
# number of arguments and what does their work
KERNEL_FUNCTIONS = {
    128: (1, kernel_query_system_time),
}


def f32(value: float) -> int:
    """A real as the 32 bits that an argument of type real is."""
    return struct.unpack("<I", struct.pack("<f", value))[0]


@dataclass
class Buffer:
    """Memory for an argument that is a pointer: the emulator gets a copy of
    the data for each call, and what the calls leave in it is compared."""

    data: bytes
    # the offsets in the data of pointers to the buffer itself: each holds an
    # offset in the buffer, and the emulator gets the address of that place
    pointers: tuple[int, ...] = ()
    # a struct format of the data (as "<3f"), to compare it value by value
    # and say which value differs; without it, the bytes are compared
    format: str | None = None
    # filled by the harness: the address in the emulator
    address: int = 0

    def values(self, data: bytes) -> tuple | bytes:
        return struct.unpack(self.format, data) if self.format else data


@dataclass
class Symbol:
    address: int
    name: str
    source: str
    # where the executable's function takes each argument: a register or "stack"
    arguments: list[str]
    # int, bool, short, real, or void. The executable's function returns a
    # real in the x87 register, or in an SSE register if it is "real:xmm0"
    returns: str


@dataclass
class Result:
    value: object
    # what the call left in each buffer of the arguments: the values of the
    # buffer's format, or the bytes
    buffers: list[tuple | bytes] = field(default_factory=list)


# with DIFFTEST_TOLERANT=1 in the environment, two reals that differ in
# their last bits are taken as the same: to tell a function that computes in
# another order from one that computes something else
TOLERANT = os.environ.get("DIFFTEST_TOLERANT") == "1"


def almost_equal(a: object, b: object) -> bool:
    """Whether two results are the same. Two reals have to be equal: the functions
    of source/ compute in single precision and in the order that the game's do."""
    if isinstance(a, float) and isinstance(b, float):
        if a == b or (a != a and b != b):
            return True
        return TOLERANT and abs(a - b) <= 2e-6 * max(abs(a), abs(b)) + 1e-9
    if isinstance(a, (tuple, list)) and isinstance(b, (tuple, list)):
        return len(a) == len(b) and all(almost_equal(x, y) for x, y in zip(a, b))
    return a == b


def read_symbols() -> dict[str, Symbol]:
    with SYMBOLS_PATH.open(newline="") as f:
        return {
            row["name"]: Symbol(
                int(row["address"], 16),
                row["name"],
                row["source"],
                [argument for argument in row["arguments"].split() if argument],
                row["returns"],
            )
            for row in csv.DictReader(f)
        }


def build_image() -> Path:
    """Compiles source/ and links it at IMAGE_ADDRESS, as an ELF file."""
    clang = shutil.which("clang")
    linker = shutil.which("ld.lld")
    if not clang or not linker:
        raise DifftestError("clang and ld.lld have to be in the PATH")
    BUILD.mkdir(parents=True, exist_ok=True)

    sources = sorted((ROOT / "source").rglob("*.c")) + [ROOT / "tools" / "difftest_libc" / "libc.c"]
    objects = []
    for source in sources:
        target = BUILD / (source.relative_to(ROOT).as_posix().replace("/", "__") + ".o")
        result = subprocess.run([clang, *COMPILE_FLAGS, "-c", source, "-o", target], capture_output=True, text=True)
        if result.returncode != 0:
            raise DifftestError(f"{source.relative_to(ROOT)} does not compile:\n{result.stderr}")
        objects.append(target)

    image = BUILD / "image.elf"
    result = subprocess.run(
        [linker, "-m", "elf_i386", "-static", f"--image-base={IMAGE_ADDRESS:#x}", "-o", image, *objects],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        raise DifftestError(f"source/ does not link:\n{result.stderr}")
    return image


class Harness:
    def __init__(self) -> None:
        if not XBE_PATH.exists():
            raise DifftestError(f"the game's executable is not at {XBE_PATH}")
        self.symbols = read_symbols()
        self.mu = Uc(UC_ARCH_X86, UC_MODE_32)
        self.kernel_call: int | None = None

        self._load_xbe(XBE_PATH.read_bytes())
        self.functions = self._load_elf(build_image().read_bytes())
        self.mu.mem_map(SCRATCH_ADDRESS, SCRATCH_SIZE)
        self.mu.mem_map(STACK_ADDRESS, STACK_SIZE)
        self.mu.mem_map(RETURN_ADDRESS, 0x1000)

        missing = [name for name in self.symbols if name not in self.functions]
        if missing:
            raise DifftestError(f"symbols/functions.csv has functions that source/ does not define: {missing}")

    # ---------- loading

    def _load_xbe(self, xbe: bytes) -> None:
        base, headers_size, image_size = struct.unpack_from("<3I", xbe, 0x104)
        section_count, sections = struct.unpack_from("<2I", xbe, 0x11C)
        (thunks,) = struct.unpack_from("<I", xbe, 0x158)

        self.mu.mem_map(base & ~0xFFF, (image_size + (base & 0xFFF) + 0xFFF) & ~0xFFF)
        self.mu.mem_write(base, xbe[:headers_size])
        for n in range(section_count):
            _, address, _, raw_address, raw_size = struct.unpack_from("<5I", xbe, sections - base + n * 0x38)
            self.mu.mem_write(address, xbe[raw_address : raw_address + raw_size])

        # the table of the kernel's functions that the executable uses: each
        # entry is the number of a function. Each gets an address of its own
        # in memory that the emulator stops at
        thunk_address = next(
            address for key in KERNEL_THUNK_KEYS if base <= (address := thunks ^ key) < base + image_size
        )
        self.mu.mem_map(KERNEL_ADDRESS, KERNEL_SIZE)
        self.mu.mem_write(KERNEL_ADDRESS, b"\xf4" * KERNEL_SIZE)
        while True:
            (entry,) = struct.unpack("<I", self.mu.mem_read(thunk_address, 4))
            if not entry:
                break
            self.mu.mem_write(thunk_address, struct.pack("<I", KERNEL_ADDRESS + (entry & 0xFFFF) * 16))
            thunk_address += 4
        self.mu.hook_add(UC_HOOK_CODE, self._kernel_hook, begin=KERNEL_ADDRESS, end=KERNEL_ADDRESS + KERNEL_SIZE - 1)

    def _kernel_hook(self, mu: Uc, address: int, size: int, user_data: object) -> None:
        self.kernel_call = (address - KERNEL_ADDRESS) // 16
        mu.emu_stop()

    def _load_elf(self, elf: bytes) -> dict[str, int]:
        """Loads the image and returns the address of each of its functions."""
        if elf[:4] != b"\x7fELF":
            raise DifftestError("the image is not an ELF file")
        program_offset, section_offset = struct.unpack_from("<2I", elf, 0x1C)
        program_size, program_count, section_size, section_count = struct.unpack_from("<4H", elf, 0x2A)

        for n in range(program_count):
            kind, offset, address, _, file_size, memory_size = struct.unpack_from(
                "<6I", elf, program_offset + n * program_size
            )
            if kind != 1 or memory_size == 0:  # PT_LOAD
                continue
            start = address & ~0xFFF
            end = (address + memory_size + 0xFFF) & ~0xFFF
            # two segments can share a page
            for page in range(start, end, 0x1000):
                try:
                    self.mu.mem_map(page, 0x1000)
                except UcError:
                    pass
            self.mu.mem_write(address, elf[offset : offset + file_size])

        functions: dict[str, int] = {}
        sections = [struct.unpack_from("<10I", elf, section_offset + n * section_size) for n in range(section_count)]
        for section in sections:
            if section[1] != 2:  # SHT_SYMTAB
                continue
            strings_offset = sections[section[6]][4]
            for at in range(section[4], section[4] + section[5], 16):
                name, value, _, info = struct.unpack_from("<3IB", elf, at)
                if info & 0xF == 2:  # STT_FUNC
                    end = elf.index(b"\0", strings_offset + name)
                    functions[elf[strings_offset + name : end].decode()] = value
        return functions

    # ---------- calling

    def _call(self, address: int, stack: list[int], registers: dict[str, int], returns: str) -> object:
        mu = self.mu
        for register in REGISTERS.values():
            mu.reg_write(register, 0)
        for name, value in registers.items():
            mu.reg_write(REGISTERS[name], value & 0xFFFFFFFF)

        # the x87 registers empty, and the control word of the console's
        # games: single precision, as Direct3D sets it
        mu.reg_write(x86.UC_X86_REG_FPCW, 0x07F)
        mu.reg_write(x86.UC_X86_REG_FPSW, 0)
        mu.reg_write(x86.UC_X86_REG_FPTAG, 0xFFFF)

        esp = STACK_ADDRESS + STACK_SIZE - 0x1000
        frame = struct.pack(f"<{len(stack) + 1}I", RETURN_ADDRESS, *(value & 0xFFFFFFFF for value in stack))
        esp -= len(frame)
        mu.mem_write(esp, frame)
        mu.reg_write(x86.UC_X86_REG_ESP, esp)
        mu.reg_write(x86.UC_X86_REG_EBP, 0)

        while True:
            self.kernel_call = None
            try:
                mu.emu_start(address, RETURN_ADDRESS, count=MAXIMUM_INSTRUCTIONS)
            except UcError as error:
                raise DifftestError(
                    f"the emulator stopped at {mu.reg_read(x86.UC_X86_REG_EIP):#010x}: {error}"
                ) from error
            if self.kernel_call is None:
                break

            # a function of the kernel: do what it does and return from it.
            # Its arguments are on the stack and it takes them off
            if self.kernel_call not in KERNEL_FUNCTIONS:
                raise DifftestError(
                    f"the function calls the kernel's function {self.kernel_call}, which the tests do not have"
                )
            argument_count, function = KERNEL_FUNCTIONS[self.kernel_call]
            esp = mu.reg_read(x86.UC_X86_REG_ESP)
            address, *kernel_arguments = struct.unpack(
                f"<{argument_count + 1}I", mu.mem_read(esp, 4 * (argument_count + 1))
            )
            mu.reg_write(x86.UC_X86_REG_EAX, function(mu, *kernel_arguments) & 0xFFFFFFFF)
            mu.reg_write(x86.UC_X86_REG_ESP, esp + 4 * (argument_count + 1))
            if address == RETURN_ADDRESS:
                mu.reg_write(x86.UC_X86_REG_EIP, RETURN_ADDRESS)
                break
        if mu.reg_read(x86.UC_X86_REG_EIP) != RETURN_ADDRESS:
            raise DifftestError("the function did not return")

        eax = mu.reg_read(x86.UC_X86_REG_EAX)
        if returns == "int":
            return eax - (1 << 32) if eax & 0x80000000 else eax
        if returns == "bool":
            return bool(eax & 0xFF)
        if returns == "short":
            return (eax & 0xFFFF) - (0x10000 if eax & 0x8000 else 0)
        if returns == "real:xmm0":
            # the low 32 bits of the register
            return struct.unpack("<f", (mu.reg_read(x86.UC_X86_REG_XMM0) & 0xFFFFFFFF).to_bytes(4, "little"))[0]
        if returns == "real":
            # the emulator numbers the x87 registers by where they are, not
            # from the top of their stack: the status word tells the top.
            # 80 bits: the mantissa, with its first bit explicit, and the sign
            # and the exponent
            top = (mu.reg_read(x86.UC_X86_REG_FPSW) >> 11) & 7
            mantissa, exponent = mu.reg_read(x86.UC_X86_REG_FP0 + top)
            value = mantissa / (1 << 63) * 2.0 ** ((exponent & 0x7FFF) - 16383) if mantissa else 0.0
            # as the 32-bit real that the game keeps
            return struct.unpack("<f", struct.pack("<f", -value if exponent & 0x8000 else value))[0]
        return None

    def _place_buffers(self, arguments: tuple) -> tuple[list[int], list[Buffer]]:
        """Writes the buffers of the arguments to the scratch memory; the arguments as integers."""
        at = SCRATCH_ADDRESS
        values, buffers = [], []
        for argument in arguments:
            if isinstance(argument, Buffer):
                argument.address = at
                data = bytearray(argument.data)
                for offset in argument.pointers:
                    struct.pack_into("<I", data, offset, struct.unpack_from("<I", data, offset)[0] + at)
                self.mu.mem_write(at, bytes(data))
                # some room after each, zeroed, for a function that reads past the end of a string
                self.mu.mem_write(at + len(argument.data), b"\0" * 64)
                at += (len(argument.data) + 64 + 15) & ~15
                values.append(argument.address)
                buffers.append(argument)
            else:
                values.append(argument)
        return values, buffers

    def _run(self, symbol: Symbol, original: bool, arguments: tuple) -> Result:
        values, buffers = self._place_buffers(arguments)
        if original:
            if len(values) != len(symbol.arguments):
                raise DifftestError(f"{symbol.name} takes {len(symbol.arguments)} arguments")
            stack = [value for value, place in zip(values, symbol.arguments) if place == "stack"]
            registers = {place: value for value, place in zip(values, symbol.arguments) if place != "stack"}
            value = self._call(symbol.address, stack, registers, symbol.returns)
        else:
            # the function of source/ returns as C functions do: a real, in
            # the x87 register
            value = self._call(self.functions[symbol.name], values, {}, symbol.returns.split(":")[0])
        return Result(
            value, [buffer.values(bytes(self.mu.mem_read(buffer.address, len(buffer.data)))) for buffer in buffers]
        )

    def original(self, name: str, *arguments: object) -> Result:
        """Calls the function of the game's executable."""
        return self._run(self.symbols[name], True, arguments)

    def ours(self, name: str, *arguments: object) -> Result:
        """Calls the function of source/."""
        return self._run(self.symbols[name], False, arguments)
