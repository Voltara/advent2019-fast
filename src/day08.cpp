#include "advent2019.h"

// Day 8: Space Image Format

static bool test(const char *p, int y, int x) {
	// Depends on safeguard zeroes added by input reader
	p += 25 * y + x;
	while (*p == '2') p += 150;
	return (*p == '1');
}

output_t day08(input_t in) {
	int part1 = 0, fewest = INT32_MAX;
	int count[256] = { };

#define COUNT(n) count[int(n)]

	for (int i = 0; i + 150 <= in.len; ) {
		COUNT('0') = COUNT('1') = COUNT('2') = 0;
		for (int k = 0; k < 150; k++, i++) {
			COUNT(in.s[i])++;
		}
		if (COUNT('0') < fewest) {
			fewest = COUNT('0');
			part1 = COUNT('1') * COUNT('2');
		}
	}

#undef COUNT

	std::string part2;
	for (int i = 0; i < 25; i += 5) {
		part2.push_back(ocr(in.s + i, test));
	}

	return { part1, part2 };
}
