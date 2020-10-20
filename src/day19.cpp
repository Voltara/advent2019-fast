#include "advent2019.h"

// Day 19: Tractor Beam

/* Threshold after which we stop refining slope approximations.
 * Must be big enough for the required precision, but small enough
 * to avoid overflow; the Intcode program evaluates a cubic expression.
 */
constexpr int CUTOFF = 1000;

namespace {

// No-frills fraction
struct frac {
	int64_t n, d;

	frac() : n(0), d(1) { }
	frac(int64_t n) : n(n), d(1) { }
	frac(int64_t n, int64_t d) : n(n), d(d) { }

	bool operator<= (const frac &o) const { return n*o.d <= d*o.n; }
	frac operator+  (const frac &o) const { return {n*o.d + d*o.n, d*o.d}; }
	frac operator-  (const frac &o) const { return {n*o.d - d*o.n, d*o.d}; }
	frac operator*  (const frac &o) const { return {n*o.n, d*o.d}; }
	frac operator/  (const frac &o) const { return {n*o.d, d*o.n}; }

	frac mediant(const frac &o) const     { return {n+o.n, d+o.d}; }
	int64_t ceil() const                  { return (n + d - 1) / d; }
	int64_t floor() const                 { return n / d; }
};

struct solver {
	decltype(read_intcode(input_t{})) V;

	/* Slopes of four lines that closely approximate the beam,
	 * and satisfy the inequality: 0 < a0 < a1 <= b0 < b1 < 1
	 *
	 * The true slopes are somewhere in the half-open intervals:
	 *     (a0, a1] - leading edge
	 *     [b0, b1) - trailing edge
	 *
	 * Most slopes will be quadratic irrationals.  In the rare
	 * cases where the slopes are rational, lattice points
	 * exactly on the edge are considered to be inside the beam.
	 */
	frac a0, a1, b0, b1;

	/* The beam is always either entirely above or below the x=y
	 * diagonal (this solution takes advantage of that fact.)
	 * To simplify things, we swap coordinates if the beam
	 * happens to be above the diagonal.
	 */
	bool swap = false;

	solver(input_t in, int x) {
		V = read_intcode(in);
		frac p;
		// Search for the beam
		std::tie(p, swap) = find_beam(x);
		// Get tight slope bounds for the leading edge...
		std::tie(a0, a1) = find_slope(frac{0,1}, p, true);
		// ...and trailing edge
		std::tie(b0, b1) = find_slope(p, frac{1,1}, false);
	}

	int part1(int size) {
		int count = 1; // (0,0)
		// Count points on or below the trailing edge of the beam
		count += count_points(size, b0);
		// Subtract points below the leading edge
		count -= count_points(size, a0);
		return count;
	}

	int part2(int size) {
		// Calculate lower bound on x-coordinate
		int y, x = ((a1 + frac{1}) / (b0 - a1) * frac{size}).ceil();

		// Scan for a solution
		for (;; x++) {
			y = (frac{x + size} * a1).ceil();
			if (size <= (b0 * frac{x}).floor() - y) {
				break;
			}
		}

		if (swap) std::swap(x, y);

		return 10000 * x + y;
	}

	// Run the Intcode program to test whether (x,y) is in the beam
	bool test_point(int x, int y) {
		if (swap) std::swap(x, y);
		cpu_t C(V);
		C.run(); *C.input = x;
		C.run(); *C.input = y;
		C.run();
		return C.output;
	}

	/* Search for any point other than (0,0) inside the beam,
	 * swapping coordinates if x < y.  Returns the fraction
	 * y/x < 1, and a boolean indicating whether a coordinate
	 * swap was performed.
	 *
	 * The parameter 'x' is how far away from the origin to search.
	 * This should not be too small, because the beam may not
	 * encompass any lattice points too near the origin.
	 */
	std::pair<frac, bool> find_beam(int x) {
		for (int offsets = 0x1324; offsets; offsets >>= 4) {
			for (int y = x - (offsets & 0xf); y > 0; y -= 4) {
				if (test_point(x, y)) {
					return { frac{y,x}, false };
				} else if (test_point(y, x)) {
					return { frac{y,x}, true };
				}
			}
		}
		return {};
	}

	/* Count lattice points between x-axis the line m*x,
	 * including points exactly on the line.  This is done
	 * using a fast Euclidean-like algorithm.
	 *
	 * The fraction m must be in reduced form.
	 *
	 * See: https://mathforum.org/library/drmath/view/73120.html
	 */
	int count_points(int x, frac m) {
		auto [ a, b ] = m;
		if (!x || !b) return 0;
		int count = a/b * x*(x+1)/2;
		a %= b;
		int q = x/b;
		count += q*(2*a*x - a*b*q + a - b + 1) / 2;
		x %= b;
		q = x*a / b;
		return count + q * x - count_points(q, frac{b,a});
	}

	/* Find good rational lower/upper bounds for the slope.
	 * Leading edge:
	 *   l = point outside beam ; h = point inside beam ; le = true
	 * Trailing edge:
	 *   l = point inside beam ; h = point outside beam ; le = false
	 */
	std::pair<frac, frac> find_slope(frac l, frac h, bool le) {
		auto T = [&](const frac &f) {
			if (f <= l) {
				return !le;
			} else if (h <= f) {
				return le;
			} else if (test_point(f.d, f.n) ^ le) {
				l = f;
				return !le;
			} else {
				h = f;
				return le;
			}
		};

		frac lo{0,1}, hi{1,1};

		/* Stern-Brocot search, with acceleration to handle
		 * pathological cases.  If the true edge is a simple
		 * rational such as 1/2, the outer bound will
		 * converge very slowly: 1/1, 2/3, 3/5, 4/7...
		 * To combat this, we double the step size every time
		 * we move in the same direction in the tree.
		 */
		for (frac mid = lo.mediant(hi); mid.d < CUTOFF; mid = lo.mediant(hi)) {
			bool t = !T(mid) ^ le;
			frac step = t ? lo : hi;
			frac &f   = t ? hi : lo;
			do {
				step = step.mediant(step);
				f = std::exchange(mid, f.mediant(step));
			} while (mid.d < CUTOFF && (T(mid) ^ le ^ t));
		}

		return { lo, hi };
	}
};

}

output_t day19(input_t in) {
	solver S{in, 42};

	int part1 = S.part1(49); // count 50x50 region
	int part2 = S.part2(99); // find 100x100 area in beam

	return { part1, part2 };
}
