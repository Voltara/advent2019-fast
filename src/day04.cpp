#include "advent2019.h"

// Day 4: Secure Container

enum {
	IS_OK = 0x1,
	IS_LO = 0x2,
	IS_HI = 0x4,
};

constexpr int N_STATES = 8192;

namespace {

struct state_t {
	uint16_t is_ok  : 1; // meets all criteria
	uint16_t is_lo  : 1; // edge case: prefix of lower bound
	uint16_t is_hi  : 1; // edge case: prefix of upper bound
	uint16_t idx    : 3; // index of current digit
	uint16_t prev   : 4; // previous digit
	uint16_t run    : 3; // run length
	uint16_t unused : 3;

	uint16_t hash() const {
		return (*(uint16_t *) this) % N_STATES;
	}
};

struct solver {
	std::vector<int16_t> Memo = { };
	const char *p_lo, *p_hi;

	solver(const char *p_lo, const char *p_hi) : p_lo(p_lo), p_hi(p_hi) {
		reset();
	}

	void reset() {
		Memo.clear();
		Memo.resize(N_STATES);
	}

	// Dynamic programming
	template<int PART1>
	int solve(state_t s) {
		if (s.idx == 6) return s.is_ok | (s.run == 2);

		auto &M = Memo[s.hash()];
		if (M) return M - 1;

		// Range of digits to check
		int lo = s.prev, hi = 9;
		if (s.is_lo) lo = std::max(lo, p_lo[s.idx] - '0');
		if (s.is_hi) hi = std::min(hi, p_hi[s.idx] - '0');

		for (int d = lo; d <= hi; d++) {
			auto n = s;

			n.is_lo &= (d == p_lo[s.idx] - '0');
			n.is_hi &= (d == p_hi[s.idx] - '0');

			// Check for admissible run of digits
			n.is_ok |= (s.run == 2) & (PART1 | (d != s.prev));

			n.idx = s.idx + 1;
			n.prev = d;
			n.run = (d == s.prev) ? (s.run + 1) : 1;

			M += solve<PART1>(n);
		}

		return M++;
	}
};

}

output_t day04(input_t in) {
	solver S{in.s, in.s + 7};

	int part1 = S.solve<1>(state_t{0,1,1,0,0,0});
	S.reset();
	int part2 = S.solve<0>(state_t{0,1,1,0,0,0});

	return { part1, part2 };
}
