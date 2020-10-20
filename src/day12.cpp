#include <x86intrin.h>
#include "advent2019.h"

// Day 12: The N-Body Problem

using axis = std::array<int, 4>;

static void step(__m64 &p, __m64 &v) {
	// Find acceleration by making pairwise comparisons 
	__m64 c0 = _mm_shuffle_pi16(p, 0x39); // 1, 2, 3, 0
	__m64 c1 = _mm_shuffle_pi16(p, 0x4e); // 2, 3, 0, 1
	__m64 c2 = _mm_shuffle_pi16(p, 0x93); // 3, 0, 1, 2

	c0 = _mm_sign_pi16(_mm_set1_pi16(1), _mm_sub_pi16(c0, p));
	c1 = _mm_sign_pi16(_mm_set1_pi16(1), _mm_sub_pi16(c1, p));
	c2 = _mm_sign_pi16(_mm_set1_pi16(1), _mm_sub_pi16(c2, p));

	// Update velocity
	v = _mm_add_pi16(v, c0);
	v = _mm_add_pi16(v, c1);
	v = _mm_add_pi16(v, c2);

	// Update position
	p = _mm_add_pi16(p, v);
}

// Simulate 1000 steps
static std::pair<axis,axis> solve_part1(axis A) {
	__m64 p = _mm_set_pi16(A[0], A[1], A[2], A[3]);
	__m64 v = _mm_setzero_si64();
	for (int i = 0; i < 1000; i++) {
		step(p, v);
	}
	p = _mm_abs_pi16(p);
	v = _mm_abs_pi16(v);

	axis P, V;
	P[0] = _mm_extract_pi16(p, 0);
	P[1] = _mm_extract_pi16(p, 1);
	P[2] = _mm_extract_pi16(p, 2);
	P[3] = _mm_extract_pi16(p, 3);
	V[0] = _mm_extract_pi16(v, 0);
	V[1] = _mm_extract_pi16(v, 1);
	V[2] = _mm_extract_pi16(v, 2);
	V[3] = _mm_extract_pi16(v, 3);

	return { P, V };
}

// Simulate until velocity of all objects is zero again,
// then double the number of steps taken
static int64_t cycle_len(axis A) {
	__m64 p = _mm_set_pi16(A[0], A[1], A[2], A[3]);
	__m64 v = _mm_setzero_si64();
	int64_t cycle = 0;
	do {
		step(p, v);
		cycle++;
	} while (_mm_cvtm64_si64(v));
	return cycle * 2;
}

output_t day12(input_t in) {
	std::array<axis, 3> A, P, V;

#define FMT "<x=%d, y=%d, z=%d>\n"
	sscanf(in.s, FMT FMT FMT FMT,
			&A[0][0], &A[1][0], &A[2][0],
			&A[0][1], &A[1][1], &A[2][1],
			&A[0][2], &A[1][2], &A[2][2],
			&A[0][3], &A[1][3], &A[2][3]);
#undef FMT

	int64_t part1 = 0;
	for (int i = 0; i < 3; i++) {
		auto [ p, v ] = solve_part1(A[i]);
		P[i] = p;
		V[i] = v;
	}
	for (int i = 0; i < 4; i++) {
		part1 += (P[0][i] + P[1][i] + P[2][i]) *
			(V[0][i] + V[1][i] + V[2][i]);
	}

	int64_t a = cycle_len(A[0]);
	int64_t b = cycle_len(A[1]);
	int64_t c = cycle_len(A[2]);
	int64_t part2 = a * b / std::gcd(a, b);
	part2 *= c / std::gcd(part2, c);

	return { part1, part2 };
}
