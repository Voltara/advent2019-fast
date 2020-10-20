#include "advent2019.h"

// Day 10: Monitoring Station

constexpr int SZ = 50;

namespace {

struct pt {
	int y, x;
	pt() : y(), x() { }
	pt(int y, int x) : y(y), x(x) { }
	pt operator + (const pt &o) const { return { y + o.y, x + o.x }; }
	pt operator - (const pt &o) const { return { y - o.y, x - o.x }; }
};

}

// Generate a list of reduced fractions with numerator and denominator not exceeding n
static std::vector<pt> fracs(int n) {
	std::vector<pt> S = { {1,1} }, F;
	for (pt L{0,1}; !S.empty(); L = S.back(), S.pop_back()) {
		F.push_back(L);
		for (pt R = S.back(); ; R = S.back()) {
			if (L.x + R.x >= n) break;
			S.emplace_back(L.y + R.y, L.x + R.x);
		}
	}
	F.emplace_back(1, 1);
	for (int i = F.size() - 2; i > 0; i--) {
		F.emplace_back(F[i].x, F[i].y);
	}
	return F;
}

output_t day10(input_t in) {
	// Read in the map, saving it both as an array of booleans and
	// a list of (y,x) coordinates
	std::vector<pt> P;
	std::vector<std::vector<bool>> G = { {} };
	for (int x = 0, y = 0; in.len--; in.s++) {
		if (*in.s == '\n') {
			G.emplace_back();
			x = 0;
			if (++y >= SZ) abort();
		} else if (*in.s != '\r') {
			G.back().push_back(*in.s == '#');
			if (*in.s == '#') P.emplace_back(y, x);
			if (++x >= SZ) abort();
		}
	}
	int DIM = G[0].size();
	G.resize(DIM);
	for (auto &g : G) g.resize(DIM);

	// Make a reduced fraction lookup table (ex: 2/4 -> 1/2)
	auto F = fracs(DIM);
	int16_t Reduced[SZ][SZ] = { };
	for (int i = 0; i < F.size(); i++) {
		auto f = F[i];
		for (auto g = f; g.y < DIM && g.x < DIM; g = g + f) {
			Reduced[g.y][g.x] = i;
		}
	}

	// Part 1: O(n^2) in the number of asteroids
	int part1 = 0, which = 0;
	std::array<std::bitset<2048>, 4> BEST = { };
	for (int i = 0; i < P.size(); i++) {
		int visible = P.size() - 1;
		std::array<std::bitset<2048>, 4> B = { };
		for (int j = 0; j < P.size(); j++) {
			if (i == j) continue;
			int dx = P[i].y - P[j].y;
			int dy = P[j].x - P[i].x;
			int quad = 0;
			if (dy > 0 && dx <= 0) {
				quad = 1;
				dx = std::exchange(dy, -dx);
			} else if (dx < 0 && dy <= 0) {
				quad = 2;
				dx = -dx, dy = -dy;
			} else if (dx >= 0 && dy < 0) {
				quad = 3;
				dy = std::exchange(dx, -dy);
			}
			int g = Reduced[dy][dx];
			visible -= B[quad].test(g);
			B[quad].set(g);
		}
		if (part1 < visible) {
			part1 = visible;
			which = i;
			BEST = B;
		}
	}

	// The Part 2 asteroid is in Quadrant 3, Layer 1
	int g = 0;
	for (int i = 200 + BEST[3].count() - part1; i; g++) {
		i -= BEST[3].test(g);
	}
	pt step = F[g - 1];
	pt f = P[which] - step;
	for (; !G[f.y][f.x]; f = f - step) { }
	int part2 = f.x * 100 + f.y;

	return { part1, part2 };
}
