#include "advent2019.h"

// Day 1: The Tyranny of the Rocket Equation

output_t day01(input_t in) {
	int part1 = 0, part2 = 0;

	for (int n = 0; in.len--; in.s++) {
		uint8_t c = *in.s - '0';
		if (c < 10) {
			// digit
			n = 10 * n + c;
		} else if (n) {
			// whitespace
			n = (n / 3) - 2;
			part1 += n;
			while ((n = (n / 3) - 2) > 0) {
				part2 += n;
			}
			n = 0;
		}
	}

	part2 += part1;

	return { part1, part2 };
}
