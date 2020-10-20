#include "advent2019.h"

// Day 24: Planet of Discord

// 2-bit SWAR saturating accumulate; second parameter can have
// 0 or 1 in each of its fields
static uint64_t sacc(uint64_t x, uint64_t a) {
	constexpr uint64_t MASK = 0x5555555555555555;
	return ((x & ~MASK) | (x & MASK) + a) ^ (a & x & (x >> 1));
}

// Extract even bits (assumes odd bits are already zero)
static uint32_t even_bits(uint64_t grid) {
	uint32_t b = grid | (grid >> 31);
	b = (b & 0x99999999) | (b & 0x22222222) << 1 | (b & 0x44444444) >> 1;
	b = (b & 0xc3c3c3c3) | (b & 0x0c0c0c0c) << 2 | (b & 0x30303030) >> 2;
	b = (b & 0xf00ff00f) | (b & 0x00f000f0) << 4 | (b & 0x0f000f00) >> 4;
	b = (b & 0xff0000ff) | (b & 0x0000ff00) << 8 | (b & 0x00ff0000) >> 8;
	return b;
}

// Count the four neighbors
static uint64_t neighbors4(uint64_t grid) {
	uint64_t n = sacc(grid << 10, grid >> 10);
	n = sacc(n, (grid & 0x0ff3fcff3fcff) << 2);
	n = sacc(n, (grid & 0x3fcff3fcff3fc) >> 2);
	return n;
}

// Apply life-or-death rule using neighbor counts in n
static uint64_t life_or_death(uint64_t grid, uint64_t n, uint64_t mask) {
	uint64_t survived =  grid & (n & ~(n >> 1));
	uint64_t born     = ~grid & (n ^  (n >> 1));
	return (survived | born) & mask;
}

// Apply part 1 rule
static uint64_t next(uint64_t grid) {
	return life_or_death(grid, neighbors4(grid), 0x1555555555555);
}

// Apply part 2 rule
static uint64_t next(uint64_t inner, uint64_t grid, uint64_t outer) {
	// Outer border masks
	constexpr uint64_t UMASK = 0x155;
	constexpr uint64_t DMASK = UMASK << 40;
	constexpr uint64_t LMASK = 0x10040100401;
	constexpr uint64_t RMASK = LMASK << 8;

	// IMASK = inner four cells, ILRMASK = inner up/down
	constexpr uint64_t IMASK   = 0x404404000;
	constexpr uint64_t IUDMASK = 0x400004000;

	uint64_t n = neighbors4(grid);

	// Outer grid neighbors
	uint64_t oud = 0, olr = 0;
	oud |= -((outer >> 14) & 1) & UMASK;
	oud |= -((outer >> 34) & 1) & DMASK;
	olr |= -((outer >> 22) & 1) & LMASK;
	olr |= -((outer >> 26) & 1) & RMASK;

	n = sacc(n, oud);
	n = sacc(n, olr);

	// Inner grid neighbors
	uint64_t iud = 0, ilr = 0;
	iud = (inner & UMASK) << 10 | (inner & DMASK) >> 10;
	ilr = (inner & LMASK) <<  2 | (inner & RMASK) >>  2;

	n = sacc(n, ( iud                 | ilr      ) & IMASK);
	n = sacc(n, ( iud >> 2            | ilr >> 10) & IMASK);
	n = sacc(n, ( iud << 2            | ilr << 10) & IMASK);
	// Additional mask needed to avoid colliding with the l/r cells
	n = sacc(n, ((iud >> 4 & IUDMASK) | ilr >> 20) & IMASK);
	n = sacc(n, ((iud << 4 & IUDMASK) | ilr << 20) & IMASK);

	return life_or_death(grid, n, 0x1555554555555);
}

output_t day24(input_t in) {
	// Represent the grid as a bit field, 2 bits per cell
	uint64_t grid = 0, b = 1;
	for (; in.len--; in.s++) {
		switch (*in.s) {
		    case '#': grid |= b;
		    case '.': b <<= 2;
		}
	}

	int part1 = 0, part2 = 0;

	// Iterate until repeat
	std::unordered_set<uint64_t> S;
	for (uint64_t g = grid; ; g = next(g)) {
		auto [ it, ok ] = S.insert(g);
		if (!ok) {
			part1 = even_bits(g);
			break;
		}
	}

	// Simulate 200 steps, trimming empty grids off the ends
	// to keep the list from growing unnecessarily
	std::vector<uint64_t> G = { grid }, N;
	for (int t = 0; t < 200 && !G.empty(); t++) {
		uint64_t prev = 0, n;

		if ((n = next(0, 0, G[0]))) N.push_back(n);

		for (int i = 1; i < G.size(); i++) {
			N.push_back(next(prev, G[i - 1], G[i]));
			prev = G[i - 1];
		}

		N.push_back(next(prev, G.back(), 0));
		if ((n = next(G.back(), 0, 0))) N.push_back(n);

		G.swap(N);
		N.clear();
	}

	// Count the bits
	for (auto g : G) {
		part2 += __builtin_popcountll(g);
	}

	return { part1, part2 };
}
