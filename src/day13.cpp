#include "advent2019.h"

// Day 13: Care Package

enum { G_EMPTY, G_WALL, G_BLOCK, G_PADDLE, G_BALL };

output_t day13(input_t in) {
	auto V = read_intcode(in);

	int part1 = 0;
	cpu_t C1(V);
	for (;;) {
		int s = C1.run();
		if (s == cpu_t::S_HLT) break;
		C1.run();
		C1.run();
		part1 += (C1.output == G_BLOCK);
	}

	// No tricks, just play the game
	int part2 = 0;
	V[0] = 2;
	cpu_t C2(V);
	for (int p = 0, b = 0; ; ) {
		int s = C2.run();
		if (s == cpu_t::S_HLT) break;
		if (s == cpu_t::S_IN) {
			*C2.input = (b < p) ? -1 : (b > p);
		} else {
			int x = C2.output;
			C2.run();
			C2.run();
			int t = C2.output;
			if (x == -1) {
				part2 = t;
			} else if (t == G_BALL) {
				b = x;
			} else if (t == G_PADDLE) {
				p = x;
			}
		}
	}

	return { part1, part2 };
}
