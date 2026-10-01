// Writes every function Ghidra found (address, size, section, name) to the CSV
// file given as the script argument, and prints the totals per section.

import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.Map;
import java.util.TreeMap;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.mem.MemoryBlock;

public class ExportFunctions extends GhidraScript {
	@Override
	public void run() throws Exception {
		File out = new File(getScriptArgs()[0]);
		Map<String, long[]> sections = new TreeMap<>();
		int count = 0;

		try (PrintWriter writer = new PrintWriter(new FileWriter(out))) {
			writer.println("address,size,section,name");
			FunctionIterator functions = currentProgram.getFunctionManager().getFunctions(true);
			while (functions.hasNext()) {
				Function function = functions.next();
				MemoryBlock block = currentProgram.getMemory().getBlock(function.getEntryPoint());
				String section = block == null ? "?" : block.getName();
				long size = function.getBody().getNumAddresses();

				writer.printf("%s,%d,%s,%s%n", function.getEntryPoint(), size, section, function.getName());
				long[] totals = sections.computeIfAbsent(section, key -> new long[2]);
				totals[0]++;
				totals[1] += size;
				count++;
			}
		}

		println("loader: " + currentProgram.getExecutableFormat());
		println("functions: " + count);
		for (Map.Entry<String, long[]> entry : sections.entrySet()) {
			println(String.format("  %-12s %6d functions %10d bytes", entry.getKey(), entry.getValue()[0],
				entry.getValue()[1]));
		}
	}
}
