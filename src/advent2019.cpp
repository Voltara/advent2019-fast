#include "advent2019.h"

std::vector<int64_t> read_intcode(input_t in) {
	std::vector<int64_t> V;
	bool neg = false;
	for (int64_t n = 0; in.len--; in.s++) {
		uint8_t c = *in.s - '0';
		if (c < 10) {
			n = 10 * n + c;
		} else if (*in.s == '-') {
			neg = true;
		} else if (*in.s != '\r') {
			if (neg) n = -n;
			V.push_back(n);
			n = 0;
			neg = false;
		}
	}
	return V;
}
