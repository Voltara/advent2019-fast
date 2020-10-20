#include "advent2019.h"

// Day 3: Crossed Wires

namespace {

struct seg {
	int dist, y, x0, x1, neg, abs_y;
	seg(int dist, int y, int x0, int x1, int neg) :
		dist(dist), y(y), x0(x0), x1(x1), neg(neg)
	{
		abs_y = (y < 0) ? -y : y;
	}
};

}

output_t day03(input_t in) {
	std::vector<seg> V, W;

	/* Make two lists of line segments; assumes both wires start
	 * parallel to each other
	 */
	int x = 0, y = 0, dist = 1;
	for (int n = 0, neg = 0; in.len--; in.s++) {
		uint8_t c = *in.s - '0';
		if (c < 10) {
			// digit
			n = 10 * n + c;
		} else if (*in.s >= 'A') {
			// direction: note D,L <= L < R,U
			neg = (*in.s <= 'L');
		} else if (*in.s != '\r') {
			// delimiter/whitespace
			if (neg) {
				V.emplace_back(dist, y, x - n, x - 1, 1);
				x -= n;
			} else {
				V.emplace_back(dist, y, x + 1, x + n, 0);
				x += n;
			}
			dist += n;

			// Rotate 90 degrees, direction to be determined later
			std::swap(x, y);
			if (*in.s != ',') {
				V.swap(W);
				x = y = 0;
				dist = 1;
			}

			n = neg = 0;
		}
	}

	int part1 = INT32_MAX, part2 = INT32_MAX;

	// O(n^2) but fast enough
	for (int i = 0; i < V.size(); i++) {
		auto &v = V[i];
		// Save factor of 2 by checking only perpendicular pairs
		for (int j = ~i & 1; j < W.size(); j += 2) {
			auto &w = W[j];
			if (w.y < v.x0) continue;
			if (w.y > v.x1) continue;
			if (v.y < w.x0) continue;
			if (v.y > w.x1) continue;
			int d = v.dist + w.dist;
			d += v.neg ? (v.x1 - w.y) : (w.y - v.x0);
			d += w.neg ? (w.x1 - v.y) : (v.y - w.x0);
			part1 = std::min(part1, v.abs_y + w.abs_y);
			part2 = std::min(part2, d);
		}
	}

	return { part1, part2 };
}
