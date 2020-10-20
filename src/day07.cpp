#include "advent2019.h"

// Day 7: Amplification Circuit

namespace {

struct stage {
	int mul, add;
	stage(int mul, int add) : mul(mul), add(add) { }
};

}

output_t day07(input_t in) {
	auto V = read_intcode(in);

	cpu_t C(V);

	/* Relies on assumptions about how the amplification works.
	 */

	// Part 1 coefficients per phase (ax+b)
	std::vector<stage> A1;
	for (int i = 0; i < 5; i++) {
		C.run(); *C.input = i;
		C.run(); *C.input = 1024;
		C.run(); A1.emplace_back(C.output / 1024, C.output % 1024);
		C.run();
	}

	// Part 2 sequence of operations per phase (*2 or +1)
	std::vector<std::vector<int>> A2;
	std::vector<int> Product(10, 1), MulMask(10);
	for (int i = 0; i < 5; i++) {
		A2.emplace_back();
		auto &a = A2.back();
		C.run(); *C.input = i + 5;
		for (int j = 0; j < 10; j++) {
			C.run(); *C.input = 0;
			C.run(); a.push_back(C.output);
		}
		C.run();
		// MulMask: Bitmask of *2 phases per iteration
		// Product: Product of all *2 in later iterations
		for (int j = 0; j < 10; j++) {
			MulMask[j] |= !a[j] << i;
			if (j && !a[j]) Product[j - 1] *= 2;
		}
	}
	for (int i = Product.size() - 1; i >= 1; i--) {
		Product[i - 1] *= Product[i];
	}

	/* For each subset of amplifiers, determine which amplifier should
	 * be added last.  For five amplifiers, 5 * 2^4 = 80 checks.
	 */
	std::vector<int> part1(32), part2(32);
	for (int m = 1; m < 32; m++) {
		for (auto idx : bits(m)) {
			int b = 1 << idx;
			int val1 = A1[idx].mul * part1[m ^ b] + A1[idx].add;
			part1[m] = std::max(part1[m], val1);

			int val2 = part2[m ^ b];
			for (int i = 0; i < 10; i++) {
				int s = __builtin_popcount(MulMask[i] & m);
				val2 += A2[idx][i] * (Product[i] << s);
			}
			part2[m] = std::max(part2[m], val2);
		}
	}

	return { part1.back(), part2.back() };
}
