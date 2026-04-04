#include <iostream>
#include <string>
#include "../include/Processor.h"
#include "../include/Preprocessor.h"

int main(int argc, char* argv[]) {

	if (argc < 2) {
		std::cerr << "Usage: ./main <filename.s> [-cycles N]\n";
		return 1;
	}

	int max_cycles = -1;
	if (argc == 4 && std::string(argv[2]) == "-cycles") {
		max_cycles = std::stoi(argv[3]);
	}

	std::string filename = argv[1];
	preprocess(filename); // comment out before submitting

	ProcessorConfig config;
	Processor cpu = Processor(config);
	
	try {
		cpu.loadProgram(filename);
	} catch (...) {
		std::cerr << "Failed to parse instruction file.\n";
		return 1;
	}

	int cycle_count = 0;
	while (cpu.step()) {
		cycle_count++;
		if (max_cycles != -1 && cycle_count == max_cycles) {
			std::cout << "\n[!] Execution halted at cycle limit: " << max_cycles << "\n";
			break;
		}
	}

	if (max_cycles == -1) {
		if (cpu.exception) {
			std::cout << "\n[+] Execution halted due to exception after " << cpu.clock_cycle << " cycles.\n";
		}
		else {
			std::cout << "\n[+] Execution complete naturally in " << cpu.clock_cycle << " cycles.\n";
		}
	}

	cpu.dumpArchitecturalState();
	for (int i=0; i<(int)cpu.Memory.size(); i++) {
		std::cout << cpu.Memory[i] << " ";
	}
	std::cout << std::endl;
	return 0;
}