#include "advent2019.h"

// Day 16: Flawed Frequency Transmission

/* C(n, k) = n choose k
 *
 * Most of the speedup has to do with C(k+99, k) mod 10
 *   C(k+99, k) % 5 = 1 ; k % 125 = 0
 *                  = 4 ; k % 125 = 25
 *                  = 0 ; otherwise
 *   C(k+99, k) % 2 = 1 ; k % 128 = 0,4,8,12,16,20,24,28
 *                  = 0 ; otherwise
 */

using int_t = int8_t;

constexpr int PHASES = 100;
constexpr int P2_REPEAT = 10000;

output_t day16(input_t in) {
	std::vector<int> V1 = { 0 };
	std::vector<int_t> V2;
	V1.reserve(in.len);
	V2.reserve(in.len);
	for (; in.len--; in.s++) {
		uint8_t c = *in.s - '0';
		if (c < 10) {
			V1.push_back(c);
			V2.push_back(c);
		}
	}
	int N = V2.size();

	// Part 1: Use prefix sum array, O(n log n) per phase
	V1.resize((V1.size() - 1) + 1);
	for (int e = 0; e < PHASES; e++) {
		for (int sum = 0, i = 0; i < V1.size(); i++) {
			sum += V1[i];
			V1[i] = sum;
		}
		for (int i = 0, save = 0; i < N; i++) {
			std::swap(save, V1[i]);

			// Add/subtract contiguous regions of the array
			int sum = 0;
			for (int k = i; k < N; ) {
				int k1 = std::min(k + (i + 1), N);
				sum = V1[k1] - V1[k] - sum;
				k = k1 + (i + 1);
			}

			V1[i] = std::exchange(save, V1[i + 1]);
			V1[i + 1] = std::abs(sum) % 10;
		}
	}
	int part1 = 0;
	for (int i = 1; i < 9; i++) part1 = 10 * part1 + V1[i];

	int part2 = 0, offset = 0;
	for (int i = 0; i < 7; i++) offset = 10 * offset + V2[i];
	int tail = P2_REPEAT * N - offset;

	// Part 2: Solve digit-by-digit combinatorially
	for (int d = 0; d < 8; d++) {
		int sum = 0, idx0 = (offset + d) % N;
		for (int ofs = 0; ofs < 32; ofs += 4) {
			int todo = tail - (d + ofs);
			int skip = todo - (todo % 83200);
			for (int i = d + ofs + skip, idx = idx0 + ofs; i < tail; i += 128, idx += 128) {
				idx -= N & -(idx >= N);
				sum ^= V2[idx];
			}
		}
		sum = (sum % 2) * 5;
		int todo = tail - (d + 0);
		int skip = todo - (todo % 16250);
		for (int i = d + skip, idx = idx0; i < tail; i += 125, idx += 125) {
			idx -= N & -(idx >= N);
			sum += 6 * V2[idx];
		}
		todo = tail - (d + 25);
		skip = todo - (todo % 16250);
		for (int i = d + 25 + skip, idx = idx0 + 25; i < tail; i += 125, idx += 125) {
			idx -= N & -(idx >= N);
			sum += 4 * V2[idx];
		}
		part2 = 10 * part2 + (sum % 10);
	}

	return { part1, part2 };
}
