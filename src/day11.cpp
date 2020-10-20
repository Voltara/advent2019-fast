#include "advent2019.h"

// Day 11: Space Police

using p2_t = bool[8][64];

static bool test(bool *p, int y, int x) {
	return p[64*y + x];
}

output_t day11(input_t in) {
	auto V = read_intcode(in);

	constexpr int DIM = 200;
	int8_t G[DIM][DIM] = { };
	int x = DIM / 2, y = DIM / 2;

	cpu_t C1(V);
	int part1 = 0, dy = -1, dx = 0;
	for (;;) {
		if (C1.run() == cpu_t::S_HLT) break;
		*C1.input = G[x][y] & 1;
		part1 += !(G[x][y] & 2);
		C1.run();
		G[x][y] = 2 | C1.output;
		C1.run();
		if (C1.output) {
			dy = std::exchange(dx, -dy);
		} else {
			dx = std::exchange(dy, -dx);
		}
		x += dx, y += dy;
		if (x < 0 || x >= DIM) abort();
		if (y < 0 || y >= DIM) abort();
	}

	p2_t H = { };
	H[0][0] = true;
	y = 0, x = 0, dy = -1, dx = 0;
	cpu_t C2(V);
	for (;;) {
		if (C1.run() == cpu_t::S_HLT) break;
		*C1.input = H[y][x];
		C1.run();
		H[y][x] = C1.output;
		C1.run();
		if (C1.output) {
			dy = std::exchange(dx, -dy);
		} else {
			dx = std::exchange(dy, -dx);
		}
		x += dx, y += dy;
		if (x < 0 || x >= 64) abort();
		if (y < 0 || y >= 8) abort();
	}

	std::string part2;
	for (int x = 1; x <= 40; x += 5) {
		part2.push_back(ocr(&H[0][x], test));
	}

	return { part1, part2 };
}
