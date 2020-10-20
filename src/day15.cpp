#include "advent2019.h"

// Day 15: Oxygen System

enum { NORTH, SOUTH, WEST, EAST };

static int move(cpu_t &C, int dir) {
	C.run();
	*C.input = dir + 1;
	C.run();
	return C.output;
}

static std::pair<int,int> solve(cpu_t &C, int back, int o2dist) {
	// max_branch: maximum depth of non-oxygen children
	// max_spread: maximum oxygen spread radius found so far
	int max_branch = 0, max_spread = 0;

	for (int dir = 0; dir < 4; dir++) {
		/* Skip if wall or backtrack.  A further optimization
		 * would be to remember the locations of walls, but the
		 * simplicity of this algorithm is too nice.
		 */
		if (dir == back || !move(C, dir)) continue;

		auto [ o, m ] = solve(C, dir ^ 1, move(C, dir) & 2);
		if (o) {
			o2dist = o + 2;
			max_spread = m;
		} else {
			max_branch = std::max(max_branch, m + 2);
		}

		move(C, dir ^ 1);
		move(C, dir ^ 1);
	}

	return { o2dist, std::max(max_spread, o2dist + max_branch) };
}

output_t day15(input_t in) {
	cpu_t C(read_intcode(in));

	auto [ o2dist, max_spread ] = solve(C, -1, 0);

	int part1 = o2dist - 2;
	int part2 = max_spread - 2;

	return { part1, part2 };
}
