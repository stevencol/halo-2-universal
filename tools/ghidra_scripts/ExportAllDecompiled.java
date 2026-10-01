// Decompiles every function of the game's code (.text) to a C file in the
// directory given as the script argument, and writes index.jsonl there: one
// JSON object per function with its address, size, name, the functions it
// calls, the number of functions that call it and the strings it refers to.

import java.io.File;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.symbol.Reference;

public class ExportAllDecompiled extends GhidraScript {
	@Override
	public void run() throws Exception {
		File directory = new File(getScriptArgs()[0]);
		directory.mkdirs();

		DecompInterface decompiler = new DecompInterface();
		decompiler.openProgram(currentProgram);
		int count = 0, failed = 0;

		try (PrintWriter index = new PrintWriter(new File(directory, "index.jsonl"), "UTF-8")) {
			FunctionIterator functions = currentProgram.getFunctionManager().getFunctions(true);
			while (functions.hasNext() && !monitor.isCancelled()) {
				Function function = functions.next();
				MemoryBlock block = currentProgram.getMemory().getBlock(function.getEntryPoint());
				if (block == null || !block.getName().equals(".text")) {
					continue;
				}

				List<String> callees = new ArrayList<>();
				for (Function callee : function.getCalledFunctions(monitor)) {
					callees.add("\"" + callee.getEntryPoint() + "\"");
				}
				Set<String> strings = new LinkedHashSet<>();
				InstructionIterator instructions = currentProgram.getListing().getInstructions(function.getBody(), true);
				while (instructions.hasNext()) {
					Instruction instruction = instructions.next();
					for (Reference reference : instruction.getReferencesFrom()) {
						Data data = getDataAt(reference.getToAddress());
						if (data != null && data.hasStringValue()) {
							strings.add(json(String.valueOf(data.getValue())));
						}
					}
				}

				DecompileResults results = decompiler.decompileFunction(function, 60, monitor);
				boolean decompiled = results.decompileCompleted();
				if (decompiled) {
					try (PrintWriter writer = new PrintWriter(new File(directory, function.getEntryPoint() + ".c"), "UTF-8")) {
						writer.print(results.getDecompiledFunction().getC());
					}
				}
				else {
					failed++;
				}

				index.println("{\"address\":\"" + function.getEntryPoint() + "\",\"size\":" +
					function.getBody().getNumAddresses() + ",\"name\":" + json(function.getName()) +
					",\"convention\":" + json(function.getCallingConventionName()) + ",\"decompiled\":" + decompiled +
					",\"callers\":" + function.getCallingFunctions(monitor).size() + ",\"callees\":[" +
					String.join(",", callees) + "],\"strings\":[" + String.join(",", strings) + "]}");
				if (++count % 1000 == 0) {
					println(count + " functions");
				}
			}
		}
		decompiler.dispose();
		println("functions: " + count + ", not decompiled: " + failed);
	}

	private static String json(String text) {
		StringBuilder builder = new StringBuilder("\"");
		for (char c : text.toCharArray()) {
			if (c == '"' || c == '\\') {
				builder.append('\\').append(c);
			}
			else if (c < 0x20 || c > 0x7E) {
				builder.append(String.format("\\u%04x", (int) c));
			}
			else {
				builder.append(c);
			}
		}
		return builder.append('"').toString();
	}
}
