#include "advent2019.h"

// Day 2: 1202 Program Alarm

static int run(std::vector<int64_t> V, int noun, int verb) {
	V[1] = noun;
	V[2] = verb;
	for (int i = 0; V[i] != 99; i += 4) {
		auto a = V[i + 1], b = V[i + 2], c = V[i + 3];
		switch (V[i]) {
		    case 1: V[c] = V[a] + V[b]; break;
		    case 2: V[c] = V[a] * V[b]; break;
		}
	}
	return V[0];
}

output_t day02(input_t in) {
	auto V = read_intcode(in);

	/* Rely on assumed behavior of the program, which evaluates
	 * the expression:
	 *     (noun * a) + verb + b
	 *
	 * This as a linear system of equations with two unknowns,
	 * which we can solve by running the program twice.
	 */
	int b = run(V, 0, 0), a = run(V, 1, 0) - b;

	int part1 = (12 * a) + 2 + b;

	int k = 19690720 - b;
	int noun = k / a;
	int verb = k - (noun * a);
	int part2 = 100 * noun + verb;

	return { part1, part2 };
}
