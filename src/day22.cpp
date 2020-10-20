#include "advent2019.h"

// Day 22: Slam Shuffle

namespace {

template<int64_t MOD>
struct shuf {
	__int128_t add, mul;
	shuf() : add(0), mul(1) { }
	shuf(int64_t add, int64_t mul) :
		add(add < 0 ? MOD + add : add), mul(mul < 0 ? MOD + mul : mul)
	{
	}

	int64_t operator () (int64_t n) const {
		return (add + mul * n) % MOD;
	}

	shuf operator * (const shuf &o) const {
		shuf r;
		r.add = (*this)(o.add);
		r.mul = mul * o.mul % MOD;
		return r;
	}

	shuf pow(int64_t e) const {
		if (e < 0) e = MOD - 1 + e;
		shuf r;
		for (shuf q = *this; e; e >>= 1, q = q * q) {
			if (e & 1) r = q * r;
		}
		return r;
	}
};

}

output_t day22(input_t in) {
	enum { OP_MUL, OP_REV, OP_ADD };

	shuf<10007> S1;
	shuf<119315717514047> S2;

	for (int n = 0, neg = 0, op = OP_MUL; in.len--; in.s++) {
		uint8_t c = *in.s - '0';
		if (c < 10) {
			n = 10 * n + c;
		} else if (*in.s == '-') {
			neg = 1;
		} else if (*in.s == 'u') { // cut
			op = OP_ADD;
		} else if (*in.s == 'k') { // new stack
			op = OP_REV;
		} else if (*in.s == '\n') {
			if (neg) n = -n;
			switch (op) {
			    case OP_MUL:
				S1 = decltype(S1){ 0, n} * S1;
				S2 = decltype(S2){ 0, n} * S2;
				break;
			    case OP_ADD:
				S1 = decltype(S1){-n, 1} * S1;
				S2 = decltype(S2){-n, 1} * S2;
				break;
			    case OP_REV:
				S1 = decltype(S1){-1,-1} * S1;
				S2 = decltype(S2){-1,-1} * S2;
				break;
			}
			n = neg = 0;
			op = OP_MUL;
		}
	}

	int64_t part1 = S1(2019);
	int64_t part2 = S2.pow(-101741582076661)(2020);

	return { part1, part2 };
}
