// Decompiles functions to C files in the directory given as the first script
// argument. Each further argument selects functions:
//   <hex address>      the function at (or containing) that address
//   const:<hex value>  every function with an instruction that holds that
//                      32-bit value (a tag such as 'head' = 68656164)

import java.io.File;
import java.io.PrintWriter;
import java.util.LinkedHashMap;
import java.util.Map;

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.mem.MemoryBlock;

public class DecompileFunctions extends GhidraScript {
	@Override
	public void run() throws Exception {
		String[] args = getScriptArgs();
		File directory = new File(args[0]);
		directory.mkdirs();

		// function entry -> why it was selected
		Map<Address, String> selected = new LinkedHashMap<>();
		for (int i = 1; i < args.length; i++) {
			if (args[i].startsWith("const:")) {
				selectByConstant(args[i].substring(6), selected);
			}
			else {
				Function function = getFunctionContaining(toAddr(Long.parseLong(args[i], 16)));
				if (function != null) {
					selected.merge(function.getEntryPoint(), args[i], (a, b) -> a + " " + b);
				}
				else {
					println("no function at " + args[i]);
				}
			}
		}

		DecompInterface decompiler = new DecompInterface();
		decompiler.openProgram(currentProgram);
		for (Map.Entry<Address, String> entry : selected.entrySet()) {
			Function function = getFunctionAt(entry.getKey());
			DecompileResults results = decompiler.decompileFunction(function, 120, monitor);
			long size = function.getBody().getNumAddresses();
			println(String.format("%s  %5d bytes  %-28s %s", entry.getKey(), size, function.getName(), entry.getValue()));
			if (!results.decompileCompleted()) {
				println("  decompile failed: " + results.getErrorMessage());
				continue;
			}
			try (PrintWriter writer = new PrintWriter(new File(directory, entry.getKey() + ".c"))) {
				writer.println("// " + function.getName() + " at " + entry.getKey() + ", " + size + " bytes; selected by " +
					entry.getValue());
				writer.print(results.getDecompiledFunction().getC());
			}
		}
		decompiler.dispose();
	}

	private void selectByConstant(String hex, Map<Address, String> selected) throws Exception {
		long value = Long.parseLong(hex, 16);
		byte[] pattern = { (byte) value, (byte) (value >> 8), (byte) (value >> 16), (byte) (value >> 24) };
		Memory memory = currentProgram.getMemory();
		int hits = 0;

		for (MemoryBlock block : memory.getBlocks()) {
			if (!block.getName().equals(".text")) {
				continue;
			}
			Address at = block.getStart();
			while (at != null && (at = memory.findBytes(at, block.getEnd(), pattern, null, true, monitor)) != null) {
				Instruction instruction = getInstructionContaining(at);
				Function function = getFunctionContaining(at);
				if (instruction != null && function != null) {
					selected.merge(function.getEntryPoint(), "const:" + hex + "@" + instruction.getAddress(),
						(a, b) -> a.contains(b) ? a : a + " " + b);
					hits++;
				}
				else if (instruction != null) {
					println("const:" + hex + " at " + instruction.getAddress() + " is in code outside any function");
				}
				at = at.add(1);
			}
		}
		println("const:" + hex + " found in " + hits + " instructions");
	}
}
