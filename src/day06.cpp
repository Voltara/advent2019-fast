#include "advent2019.h"

// Day 6: Universal Orbit Map

namespace {

struct orb_t {
	int parent = -1;
	int depth = -1;
	bool mark = false;
	orb_t() { }
};

}

// Find the depth of a given node in the tree
static int get_depth(std::vector<orb_t> &O, int idx) {
	if (idx == -1) return -1;
	auto &o = O[idx];
	if (o.depth == -1) o.depth = 1 + get_depth(O, o.parent);
	return o.depth;
}

static constexpr uint16_t str2id(const char *s) {
	return ((s[0] - 'A') * 36 + (s[1] - 'A')) * 36 + (s[2] - 'A');
}

output_t day06(input_t in) {
	std::vector<int16_t> Idx(65536);

	std::vector<orb_t> O = { orb_t{} };
	O.reserve(1500);

	for (uint16_t x = 0, y = 0; in.len--; in.s++) {
		char c = *in.s;
		if (c >= 'A') {
			x = 36 * x + (c - 'A');
		} else if (c >= '0') {
			x = 36 * x + (c - '0' + 26);
		} else if (c == ')') {
			std::swap(x, y);
		} else if (c != '\r') {
			// Add new orbits to index
			for (auto id : { x, y }) {
				if (Idx[id]) continue;
				Idx[id] = O.size();
				O.emplace_back();
			}
			O[Idx[x]].parent = Idx[y];
			x = y = 0;
		}
	}

	// Part 1: O(n) sum of nesting depths of all orbits
	int part1 = 0;
	for (int i = 0; i < O.size(); i++) {
		O[i].depth = get_depth(O, i);
		part1 += O[i].depth;
	}

	int you = Idx[str2id("YOU")], san = Idx[str2id("SAN")];

	// Part 2: O(n) distance using lowest common ancestor
	int part2 = O[you].depth + O[san].depth - 2;
	for (; you != -1; you = O[you].parent) O[you].mark = true;
	for (; !O[san].mark; san = O[san].parent) { }
	part2 -= O[san].depth * 2;

	return { part1, part2 };
}
