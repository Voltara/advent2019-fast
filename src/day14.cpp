#include "advent2019.h"

// Day 14: Space Stoichiometry

using ID = std::string;

namespace {

struct pile {
	ID id;
	int64_t n;
	pile() : id(), n() { }
	pile(ID id, int64_t n) : id(id), n(n) { }
};

struct recipe {
	std::vector<pile> in;
	pile out;
	bool mark;
	recipe() : in(), out(), mark() { }
};

}

using rmap_t = std::unordered_map<ID, recipe>;

// Topological sort, DFS starting at 'id'
static void topo(std::vector<ID> &T, rmap_t &Recipe, ID id) {
	auto &r = Recipe[id];
	if (r.mark) return;
	r.mark = true;
	for (auto p : r.in) topo(T, Recipe, p.id);
	T.push_back(id);
}

// Cost to synthesize 'fuel' units of fuel
static int64_t cost(const std::vector<ID> &T, const rmap_t &Recipe, int64_t fuel) {
	std::unordered_map<ID, int64_t> Need;
	Need["FUEL"] = fuel;
	for (auto id : T) {
		if (id == "ORE") break;
		auto &r = Recipe.find(id)->second;
		int64_t n = (Need[id] + r.out.n - 1) / r.out.n;
		for (auto &p : r.in) {
			Need[p.id] += n * p.n;
		}
	}
	return Need["ORE"];
}

output_t day14(input_t in) {
	rmap_t Recipe;

	std::string id;
	recipe R;
	for (int n = 0; in.len--; in.s++) {
		uint8_t c = *in.s - '0';
		if (c < 10) {
			n = 10 * n + c;
		} else if (*in.s >= 'A') {
			id.push_back(*in.s);
		} else if (*in.s == ',' || *in.s == '=') {
			R.in.emplace_back(id, n);
			n = 0, id.clear();
		} else if (*in.s == '\n') {
			R.out = { id, n };
			Recipe[id] = R;
			n = 0, id.clear(), R = { };
		}
	}

	std::vector<ID> T;
	topo(T, Recipe, "FUEL");
	std::reverse(T.begin(), T.end());

	// One unit of fuel
	int64_t part1 = cost(T, Recipe, 1);

	// Interpolation search for part 2
	int64_t supply = 1000000000000;
	int64_t lo = 1, hi = (supply / part1) * 4;
	int64_t lo_ore = part1, hi_ore = cost(T, Recipe, hi);
	while (lo < hi - 1) {
		double p = double(supply - lo_ore) / (hi_ore - lo_ore);

		int64_t fuel = lo + p * (hi - lo);
		fuel = std::min(fuel, hi - 1);
		fuel = std::max(fuel, lo + 1);

		int64_t ore = cost(T, Recipe, fuel);

		if (ore == supply) {
			lo = fuel;
		} else if (ore < supply) {
			lo = fuel, lo_ore = ore;
		} else {
			hi = fuel, hi_ore = ore;
		}
	}
	int64_t part2 = lo;

	return { part1, part2 };
}
