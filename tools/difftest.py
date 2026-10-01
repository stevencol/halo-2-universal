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

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from unicorn import UC_ARCH_X86, UC_HOOK_CODE, UC_MODE_32, Uc, UcError
from unicorn import x86_const as x86

ROOT = Path(__file__).resolve().parent.parent
XBE_PATH = ROOT / "disc" / "default.xbe"
SYMBOLS_PATH = ROOT / "symbols" / "functions.csv"
GLOBALS_PATH = ROOT / "symbols" / "globals.csv"
BUILD = ROOT / "build" / "difftest"

# the emulator's memory
IMAGE_ADDRESS = 0x20000000  # source/, compiled
SCRATCH_ADDRESS = 0x60000000  # the buffers of the arguments
SCRATCH_SIZE = 0x01000000
STACK_ADDRESS = 0x70000000
STACK_SIZE = 0x00100000
STACK_DIRTY_SIZE = 0x4000  # below the arguments, filled before each call
RETURN_ADDRESS = 0x7FFF0000  # a call returns here, where the emulator stops
KERNEL_ADDRESS = 0x7FFE0000  # the functions of the console's kernel, which the emulator does not have
KERNEL_SIZE = 0x4000
MOCK_ADDRESS = 0x7FFD0000  # the functions that a test stands in for
MOCK_SIZE = 0x4000
# in that memory, from here on: the functions of the game that source/ calls and does not have yet
EXTERNAL_OFFSET = 0x800
# where the console maps its memory: the game takes blocks of it by address.
# A test that uses it has to fill it, as it stays from one call to the next
PHYSICAL_ADDRESS = 0x80000000
PHYSICAL_SIZE = 0x00800000

KERNEL_THUNK_KEYS = (0x5B6D40B6, 0xEFB1F152)
MAXIMUM_INSTRUCTIONS = 200_000_000
# of the functions that a test stands in for, in one call: a loop that calls
# one each time around starts the count of instructions again each time
MAXIMUM_CALLS = 10_000

REGISTERS = {
    "eax": x86.UC_X86_REG_EAX,
    "ebx": x86.UC_X86_REG_EBX,
    "ecx": x86.UC_X86_REG_ECX,
    "edx": x86.UC_X86_REG_EDX,
    "esi": x86.UC_X86_REG_ESI,
    "edi": x86.UC_X86_REG_EDI,
    # a real argument, as its 32 bits, in the low part of an SSE register
    "xmm0": x86.UC_X86_REG_XMM0,
    "xmm1": x86.UC_X86_REG_XMM1,
    "xmm2": x86.UC_X86_REG_XMM2,
    "xmm3": x86.UC_X86_REG_XMM3,
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
    # a call to a function stays a call, so that a test can stand in for it
    "-fno-inline",
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


def external_system_time(harness: object, arguments: list[int]) -> tuple:
    """GetSystemTimeAsFileTime: the same fixed time."""
    kernel_query_system_time(harness.mu, arguments[0])
    return 0, ()


# the functions of the console's libraries that a test does not have to stand
# in for: what each does here
DEFAULT_EXTERNALS = {
    "GetSystemTimeAsFileTime": external_system_time,
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
class Pointer:
    """An argument or the value of a global: an address inside a buffer of the same call."""

    buffer: Buffer
    offset: int


@dataclass
class FunctionPointer:
    """An argument that is a function of symbols/functions.csv: the executable's
    function gets the executable's, and the function of source/ gets that of source/."""

    name: str


@dataclass
class GlobalData:
    """The value of a global that has pointers to itself, as a table whose
    entries are in a list: the bytes of the global, which is not at the same
    address in the game and in source/."""

    data: bytes
    # the offsets in the data of pointers to the global itself: each holds an
    # offset in the global, and the emulator gets the address of that place
    pointers: tuple[int, ...] = ()
    # the offsets in the data of pointers to functions of symbols/functions.csv, by name
    functions: dict[int, str] = field(default_factory=dict)
    # the offsets where the call can leave a pointer to the global or to a
    # function: what is there is compared as the place it points to
    places: tuple[int, ...] = ()
    # takes what the call left in the global and returns what to compare of
    # it: to leave out what the game copies from memory that it never set
    clean: object = None


@dataclass
class GlobalPointer:
    """An argument or the value of a global: the address of a global of
    symbols/globals.csv, or of a place in it."""

    name: str
    offset: int = 0


@dataclass
class Memory:
    """What a test puts at an address of the emulator's memory before a call
    (the `memory` of a call, by address) and has compared after it: for what
    the game reaches through a pointer of a global."""

    data: bytes
    # the offsets where the call can leave a pointer to a global or to a
    # function: what is there is compared as the place it points to
    places: tuple[int, ...] = ()


@dataclass
class GlobalRange:
    """The value of a global that the test does not set: only tells its size,
    so that a pointer to it is compared as the place it points to."""

    size: int


class StopCall(Exception):
    """What a function that stands in for another raises to end the call there:
    for a function that never returns."""


@dataclass
class MockCall:
    """A call to a mock: the number of the mock, the registers that it takes arguments in, and its stack arguments."""

    mock: int
    registers: dict[str, int]
    stack: list[int]


@dataclass
class Mock:
    """A function that the test stands in for, at the address mock_address(n) for
    the n-th mock of a call: what a pointer to a function in the data of a test
    points to. Both functions call it the same way, and the calls are compared.

    The handler takes the call and returns what the function returns.
    """

    handler: object
    # the arguments on the stack, and whether the function takes them off
    stack_arguments: int = 0
    callee_cleans: bool = True
    # the registers that the function takes arguments in
    registers: tuple[str, ...] = ()


def mock_address(n: int) -> int:
    return MOCK_ADDRESS + n * 16


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
    # "thiscall" for a function of source/ that takes its first argument in ecx
    convention: str = ""


@dataclass
class Result:
    value: object
    # what the call left in each buffer of the arguments: the values of the
    # buffer's format, or the bytes
    buffers: list[tuple | bytes] = field(default_factory=list)
    # the calls to the mocks, in order: the mock, its registers and its stack arguments
    calls: list[tuple] = field(default_factory=list)


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
                row.get("convention") or "",
            )
            for row in csv.DictReader(f)
        }


def external_names() -> list[str]:
    """The functions of symbols/functions.csv with no source: not decompiled, but called by source/."""
    return [name for name, symbol in read_symbols().items() if not symbol.source]


def external_address(n: int) -> int:
    return MOCK_ADDRESS + EXTERNAL_OFFSET + n * 16


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

    # the functions that source/ calls and does not have yet go to addresses
    # that the emulator stops at
    externals = [f"--defsym={name}={external_address(n):#x}" for n, name in enumerate(external_names())]

    image = BUILD / "image.elf"
    result = subprocess.run(
        [linker, "-m", "elf_i386", "-static", f"--image-base={IMAGE_ADDRESS:#x}", *externals, "-o", image, *objects],
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
        self.mu.mem_map(PHYSICAL_ADDRESS, PHYSICAL_SIZE)

        self.mock_call: int | None = None
        self._mocks: list[Mock] = []
        self._calls: list[tuple] = []
        self._ranges: list[tuple[str, int, int]] = []
        self._function_names: dict[int, str] = {}
        self.mu.mem_map(MOCK_ADDRESS, MOCK_SIZE)
        self.mu.mem_write(MOCK_ADDRESS, b"\xf4" * MOCK_SIZE)
        self.mu.hook_add(UC_HOOK_CODE, self._mock_hook, begin=MOCK_ADDRESS, end=MOCK_ADDRESS + MOCK_SIZE - 1)

        # the functions that a test stands in for, by name: those that are
        # not decompiled, which source/ calls at an address of the mocks'
        # memory, and any other of symbols/functions.csv. The emulator stops
        # where each starts, in the game and in source/
        self.externals = external_names()
        self._external_handlers: dict[str, object] = {}
        self.external_call: str | None = None
        # how many bytes of arguments the game's function takes off the
        # stack when it returns
        self._external_cleanup: dict[str, int] = {}
        # the sizes of the game's functions, if Ghidra's list of them is there
        self._game_function_sizes: dict[int, int] = {}
        sizes_path = ROOT / "ghidra" / "functions.csv"
        if sizes_path.exists():
            with sizes_path.open(newline="") as f:
                self._game_function_sizes = {int(row["address"], 16): int(row["size"]) for row in csv.DictReader(f)}
        self._external_cleanup_ours: dict[str, int] = {}

        missing = [name for name, symbol in self.symbols.items() if symbol.source and name not in self.functions]
        if missing:
            raise DifftestError(f"symbols/functions.csv has functions that source/ does not define: {missing}")

        # the global variables: the address of each in the executable
        with GLOBALS_PATH.open(newline="") as f:
            self.globals = {row["name"]: int(row["address"], 16) for row in csv.DictReader(f)}
        missing = [name for name in self.globals if name not in self.variables]
        if missing:
            raise DifftestError(f"symbols/globals.csv has variables that source/ does not define: {missing}")

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

    def _mock_hook(self, mu: Uc, address: int, size: int, user_data: object) -> None:
        if address >= MOCK_ADDRESS + EXTERNAL_OFFSET:
            # source/ calls a function that it does not have
            self.external_call = self.externals[(address - MOCK_ADDRESS - EXTERNAL_OFFSET) // 16]
            self._external_side = "ours"
        else:
            self.mock_call = (address - MOCK_ADDRESS) // 16
        mu.emu_stop()

    def _hook_external(self, name: str) -> None:
        """Makes the emulator stop where a function starts, the first time that a test stands in for it."""
        if name in self._external_cleanup:
            return
        if name not in self.symbols:
            raise DifftestError(f"symbols/functions.csv does not have '{name}'")
        address = self.symbols[name].address
        self._external_cleanup[name] = self._game_cleanup(name, address)
        self.mu.hook_add(UC_HOOK_CODE, self._external_hook, user_data=(name, "original"), begin=address, end=address)
        if name in self.functions:
            address = self.functions[name]
            # a function of source/ that takes its arguments off the stack, as the game's does
            for instruction in Cs(CS_ARCH_X86, CS_MODE_32).disasm(bytes(self.mu.mem_read(address, 0x2000)), address):
                if instruction.mnemonic == "ret":
                    self._external_cleanup_ours[name] = int(instruction.op_str, 0) if instruction.op_str else 0
                    break
            self.mu.hook_add(UC_HOOK_CODE, self._external_hook, user_data=(name, "ours"), begin=address, end=address)

    def _game_cleanup(self, name: str, address: int, depth: int = 0) -> int:
        """How many bytes of arguments a function of the game takes off the stack when it returns."""
        # as far as the function goes, if its size is known: a function can
        # end in a jump to another, which is the one that returns
        size = self._game_function_sizes.get(address, 0x2000)
        last = None
        for instruction in Cs(CS_ARCH_X86, CS_MODE_32).disasm(bytes(self.mu.mem_read(address, size)), address):
            if instruction.address == address and instruction.mnemonic == "jmp" and "[" in instruction.op_str:
                # only a jump to a function of the kernel, which takes its arguments off the stack
                return 4 * self.symbols[name].arguments.count("stack")
            if instruction.mnemonic == "ret":
                return int(instruction.op_str, 0) if instruction.op_str else 0
            if instruction.mnemonic != "int3":
                last = instruction
        if last is not None and last.mnemonic == "jmp" and last.op_str.startswith("0x") and depth < 8:
            return self._game_cleanup(name, int(last.op_str, 16), depth + 1)
        # the size is not always all of the function: the first return after where it starts
        for instruction in Cs(CS_ARCH_X86, CS_MODE_32).disasm(bytes(self.mu.mem_read(address, 0x2000)), address):
            if instruction.mnemonic == "ret":
                return int(instruction.op_str, 0) if instruction.op_str else 0
        raise DifftestError(f"no return found in the game's function {name}")

    def _external_hook(self, mu: Uc, address: int, size: int, user_data: tuple) -> None:
        # a function that a test stands in for; it runs if this test does not
        name, side = user_data
        if name in self._external_handlers and name != self._running:
            self.external_call = name
            self._external_side = side
            mu.emu_stop()

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
        self.variables: dict[str, int] = {}
        sections = [struct.unpack_from("<10I", elf, section_offset + n * section_size) for n in range(section_count)]
        for section in sections:
            if section[1] != 2:  # SHT_SYMTAB
                continue
            strings_offset = sections[section[6]][4]
            for at in range(section[4], section[4] + section[5], 16):
                name, value, _, info = struct.unpack_from("<3IB", elf, at)
                if info & 0xF in (1, 2):  # STT_OBJECT, STT_FUNC
                    end = elf.index(b"\0", strings_offset + name)
                    table = functions if info & 0xF == 2 else self.variables
                    table[elf[strings_offset + name : end].decode()] = value
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
        # what the function finds in its stack before it writes there is not
        # zeros, so that a variable that only one of the two sets shows
        mu.mem_write(esp - STACK_DIRTY_SIZE, b"\xcd" * STACK_DIRTY_SIZE)
        mu.mem_write(esp, frame)
        mu.reg_write(x86.UC_X86_REG_ESP, esp)
        mu.reg_write(x86.UC_X86_REG_EBP, 0)

        stopped = False
        calls = 0
        while True:
            calls += 1
            if calls > MAXIMUM_CALLS:
                raise DifftestError("the function calls others without end")
            self.kernel_call = None
            self.mock_call = None
            self.external_call = None
            try:
                mu.emu_start(address, RETURN_ADDRESS, count=MAXIMUM_INSTRUCTIONS)
            except UcError as error:
                raise DifftestError(
                    f"the emulator stopped at {mu.reg_read(x86.UC_X86_REG_EIP):#010x}: {error}"
                ) from error

            if self.external_call is not None:
                # a function of the game that the test stands in for. The
                # game's code gives it its arguments where the game's function
                # takes them; source/ gives them all on the stack
                name = self.external_call
                if name not in self._external_handlers:
                    raise DifftestError(f"source/ calls {name}, which is not decompiled and the test does not stand in for")
                symbol = self.symbols[name]
                esp = mu.reg_read(x86.UC_X86_REG_ESP)
                (address,) = struct.unpack("<I", mu.mem_read(esp, 4))
                if self._external_side == "original":
                    stack_count = symbol.arguments.count("stack")
                    stack = iter(struct.unpack(f"<{stack_count}I", mu.mem_read(esp + 4, 4 * stack_count)))
                    arguments = [
                        next(stack) if place == "stack" else mu.reg_read(REGISTERS[place]) & 0xFFFFFFFF
                        for place in symbol.arguments
                    ]
                    cleanup = self._external_cleanup[name]
                else:
                    count = len(symbol.arguments)
                    arguments = list(struct.unpack(f"<{count}I", mu.mem_read(esp + 4, 4 * count)))
                    cleanup = self._external_cleanup_ours.get(name, 0)
                try:
                    result = self._external_handlers[name](self, arguments)
                except StopCall:
                    self._calls.append((name, "stopped"))
                    stopped = True
                    break
                # the handler can give what to compare of the call, when its
                # arguments have addresses that are not the same in both
                value, record = (
                    result if isinstance(result, tuple) else (result, tuple(map(self.normalize, arguments)))
                )
                self._calls.append((name, record))
                mu.reg_write(x86.UC_X86_REG_EAX, (value or 0) & 0xFFFFFFFF)
                mu.reg_write(x86.UC_X86_REG_ESP, esp + 4 + cleanup)
                if address == RETURN_ADDRESS:
                    mu.reg_write(x86.UC_X86_REG_EIP, RETURN_ADDRESS)
                    break
                continue

            if self.mock_call is not None:
                # a function that the test stands in for: its work is the
                # test's, and it returns as the function it stands for does
                if self.mock_call >= len(self._mocks):
                    raise DifftestError(f"the function calls the mock {self.mock_call}, which the test does not give")
                mock = self._mocks[self.mock_call]
                esp = mu.reg_read(x86.UC_X86_REG_ESP)
                address, *stack_arguments = struct.unpack(
                    f"<{mock.stack_arguments + 1}I", mu.mem_read(esp, 4 * (mock.stack_arguments + 1))
                )
                call = MockCall(
                    self.mock_call,
                    {name: mu.reg_read(REGISTERS[name]) for name in mock.registers},
                    stack_arguments,
                )
                self._calls.append(
                    (
                        call.mock,
                        tuple((name, self.normalize(value)) for name, value in call.registers.items()),
                        tuple(map(self.normalize, call.stack)),
                    )
                )
                try:
                    result = mock.handler(call)
                except StopCall:
                    stopped = True
                    break
                mu.reg_write(x86.UC_X86_REG_EAX, (result or 0) & 0xFFFFFFFF)
                mu.reg_write(x86.UC_X86_REG_ESP, esp + 4 + (4 * mock.stack_arguments if mock.callee_cleans else 0))
                if address == RETURN_ADDRESS:
                    mu.reg_write(x86.UC_X86_REG_EIP, RETURN_ADDRESS)
                    break
                continue

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
        if stopped:
            return None
        if mu.reg_read(x86.UC_X86_REG_EIP) != RETURN_ADDRESS:
            raise DifftestError("the function did not return")

        eax = mu.reg_read(x86.UC_X86_REG_EAX)
        if returns == "int":
            return eax - (1 << 32) if eax & 0x80000000 else eax
        if returns == "pointer":
            return self.normalize(eax)
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

    def normalize(self, value: int) -> object:
        """A value that can be a pointer to a global that has pointers to itself
        (difftest.GlobalData) or to a function of source/, as what it points
        to: the game and source/ do not have them at the same address."""
        for key, address, size in self._ranges:
            if address <= value < address + size:
                return ("pointer", key, value - address)
        # the end of a global, as a pointer past its last entry
        for key, address, size in self._ranges:
            if value == address + size:
                return ("pointer", key, size)
        name = self._function_names.get(value)
        return ("function", name) if name else value

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

    def _run(
        self,
        symbol: Symbol,
        original: bool,
        arguments: tuple,
        globals_: dict[str, object] | None,
        mocks: list[Mock] | None = None,
        extra: list[Buffer] | None = None,
        externals: dict[str, object] | None = None,
        memory: dict[int, Memory] | None = None,
    ) -> Result:
        # the buffers of the globals go after those of the arguments, and the
        # extra ones after them
        globals_ = globals_ or {}
        self._mocks = mocks or []
        self._external_handlers = {**DEFAULT_EXTERNALS, **(externals or {})}
        for name in self._external_handlers:
            self._hook_external(name)
        self._running = symbol.name
        self._calls = []
        values, buffers = self._place_buffers((*arguments, *globals_.values(), *(extra or ())))
        values = values[: len(arguments) + len(globals_)]
        # for the functions that stand in for others: where each global of
        # symbols/globals.csv is in the side that runs
        self.global_addresses = self.globals if original else {name: self.variables[name] for name in self.globals}
        # where each function is in the side that runs, to tell a pointer to one
        self._function_names = {
            (symbol.address if original else self.functions[name]): name
            for name, symbol in self.symbols.items()
            if symbol.source
        }
        # and those that source/ does not have, which it calls at an address of the mocks' memory
        for n, name in enumerate(self.externals):
            self._function_names[self.symbols[name].address if original else external_address(n)] = name
        # the globals that have pointers to themselves: (key, address, size)
        self._ranges = []
        for n, value in enumerate(values):
            if isinstance(value, Pointer):
                values[n] = value.buffer.address + value.offset
            elif isinstance(value, FunctionPointer):
                values[n] = self.symbols[value.name].address if original else self.functions[value.name]
            elif isinstance(value, GlobalPointer):
                if value.name not in self.globals:
                    raise DifftestError(f"symbols/globals.csv does not have '{value.name}'")
                values[n] = self.global_addresses[value.name] + value.offset
        global_addresses: dict[str, tuple] = {}
        for key, value in zip(globals_, values[len(arguments) :]):
            # "name+offset" is a field of a global structure
            name, _, offset = key.partition("+")
            if name not in self.globals:
                raise DifftestError(f"symbols/globals.csv does not have '{name}'")
            address = (self.globals[name] if original else self.variables[name]) + int(offset or "0", 0)
            # bytes are the global itself, of any size; anything else is a
            # 32-bit value. None is a global that the test only wants
            # compared after the call
            places: tuple[int, ...] | None = None
            clean = None
            if isinstance(value, GlobalRange):
                self._ranges.append((key, address, value.size))
                continue
            if isinstance(value, GlobalData):
                clean = value.clean
                data = bytearray(value.data)
                for offset in value.pointers:
                    struct.pack_into("<I", data, offset, struct.unpack_from("<I", data, offset)[0] + address)
                for offset, function in value.functions.items():
                    struct.pack_into(
                        "<I", data, offset, self.symbols[function].address if original else self.functions[function]
                    )
                places = value.places
                self._ranges.append((key, address, len(data)))
                value = bytes(data)
            elif not isinstance(value, bytes):
                value = struct.pack("<I", (value or 0) & 0xFFFFFFFF)
            global_addresses[key] = (address, len(value), places, clean)
            self.mu.mem_write(address, value)
        values = values[: len(arguments)]
        for address, block in (memory or {}).items():
            self.mu.mem_write(address, block.data)

        if original:
            if len(values) != len(symbol.arguments):
                raise DifftestError(f"{symbol.name} takes {len(symbol.arguments)} arguments")
            stack = [value for value, place in zip(values, symbol.arguments) if place == "stack"]
            registers = {place: value for value, place in zip(values, symbol.arguments) if place != "stack"}
            value = self._call(symbol.address, stack, registers, symbol.returns)
        else:
            # the function of source/ returns as C functions do: a real, in
            # the x87 register
            registers = {}
            if symbol.convention == "thiscall":
                registers = {"ecx": values[0]}
                values = values[1:]
            value = self._call(self.functions[symbol.name], values, registers, symbol.returns.split(":")[0])
        # what the call left in the globals of the test is compared with the calls
        for key, (address, size, places, clean) in global_addresses.items():
            data = bytearray(self.mu.mem_read(address, size))
            if clean:
                data = bytearray(clean(bytes(data)))
            if places is not None:
                # the pointers apart, as what they point to
                pointers = []
                for offset in places:
                    pointers.append(self.normalize(struct.unpack_from("<I", data, offset)[0]))
                    struct.pack_into("<I", data, offset, 0)
                self._calls.append(("global", key, bytes(data), tuple(pointers)))
            elif size == 4:
                self._calls.append(("global", key, self.normalize(struct.unpack("<I", data)[0])))
            else:
                self._calls.append(("global", key, bytes(data)))
        for address, block in (memory or {}).items():
            data = bytearray(self.mu.mem_read(address, len(block.data)))
            pointers = []
            for offset in block.places:
                pointers.append(self.normalize(struct.unpack_from("<I", data, offset)[0]))
                struct.pack_into("<I", data, offset, 0)
            self._calls.append(("memory", address, bytes(data), tuple(pointers)))
        return Result(
            value,
            [buffer.values(bytes(self.mu.mem_read(buffer.address, len(buffer.data)))) for buffer in buffers],
            self._calls,
        )

    def original(self, name: str, *arguments: object, **options: object) -> Result:
        """Calls the function of the game's executable.

        `globals` gives global variables of symbols/globals.csv a value for
        the call: an integer, a buffer that the variable points to, or bytes
        that are the variable itself. `mocks` are the functions that the test
        stands in for through a pointer. `extra` are buffers that are no
        argument, for the mocks to use. `externals` are functions of
        symbols/functions.csv, by name, which the test stands in for,
        decompiled or not: each takes the harness and the arguments, in the
        order of symbols/functions.csv, and returns what the function returns.
        """
        return self._run(self.symbols[name], True, arguments, *self._options(options))

    def ours(self, name: str, *arguments: object, **options: object) -> Result:
        """Calls the function of source/."""
        return self._run(self.symbols[name], False, arguments, *self._options(options))

    @staticmethod
    def _options(options: dict) -> tuple:
        return (
            options.get("globals"),
            options.get("mocks"),
            options.get("extra"),
            options.get("externals"),
            options.get("memory"),
        )
