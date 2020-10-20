#include "advent2019.h"

// Day 9: Sensor Boost

output_t day09(input_t in) {
	auto V = read_intcode(in);

	int64_t part1 = 0;
	cpu_t C1(V);
	C1.run(); *C1.input = 1;
	do {
		C1.run(); part1 = C1.output;
	} while (!part1);

	int64_t part2 = 0;
	cpu_t C2(V);
	C2.run(); *C2.input = 2;
	do {
		C2.run(); part2 = C2.output;
	} while (!part2);

	return { part1, part2 };
}
