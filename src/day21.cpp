#include "advent2019.h"

// Day 21: Springdroid Adventure

output_t day21(input_t in) {
	auto V = read_intcode(in);

	int part1 = 0, part2 = 0;

	std::string P1 =
		"NOT C J\n"
		"NOT A T\n"
		"OR T J\n"
		"AND D J\n"
		"WALK\n";
	cpu_t C1(V);
	for (auto c = P1.begin(); ; ) {
		int s = C1.run();
		if (s == cpu_t::S_IN) {
			*C1.input = *c++;
		} else if (C1.output >= 128) {
			part1 = C1.output;
			break;
		}
	}

	std::string P2 =
		"OR B J\n"
		"AND C J\n"
		"NOT J J\n"
		"AND H J\n"
		"NOT A T\n"
		"OR T J\n"
		"AND D J\n"
		"RUN\n";
	cpu_t C2(V);
	for (auto c = P2.begin(); ; ) {
		int s = C2.run();
		if (s == cpu_t::S_IN) {
			*C2.input = *c++;
		} else if (C2.output >= 128) {
			part2 = C2.output;
			break;
		}
	}

	return { part1, part2 };
}
