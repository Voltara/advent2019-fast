#ifndef _ADVENT2019_H
#define _ADVENT2019_H

#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <bitset>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cstring>
#include <numeric>

struct input_t {
	char *s;
	ssize_t len;
};

struct output_t {
	std::string part1;
	std::string part2;

	template<typename T1, typename T2>
	output_t(T1 p1, T2 p2) {
		set_part1(p1);
		set_part2(p2);
	}

	void set_part1(int64_t n) { set(part1, n); }
	void set_part2(int64_t n) { set(part2, n); }

	void set_part1(const std::string &s) { part1 = s; }
	void set_part2(const std::string &s) { part2 = s; }

	static void set(std::string &s, int64_t n) {
		char buf[64];
		sprintf(buf, "%ld", n);
		s = buf;
	}
};

// Iterate over bits using range-for syntax:
//   for (int i : bits(mask))
struct bits {
	int mask;
	bits(unsigned mask) : mask(mask)      { }
	bits& operator++ ()                   { mask &= mask - 1; return *this; }
	bool operator!= (const bits &o) const { return mask != o.mask; }
	int operator* () const                { return __builtin_ctz(mask); }
	bits begin() const                    { return mask; }
	bits end() const                      { return 0; }
};

// Character recognition
template<typename T, typename F>
static char ocr(T *p, F test) {
	if (test(p, 0, 3)) {
		if (test(p, 5, 3)) {
			if (test(p, 0, 2)) {
				return test(p, 1, 0) ? 'E' : 'Z';
			} else {
				return test(p, 1, 2) ? 'K' : 'H';
			}
		} else {
			if (test(p, 0, 2)) {
				return test(p, 0, 0) ? 'F' : 'J';
			} else {
				return 'U';
			}
		}
	} else {
		if (test(p, 5, 3)) {
			if (test(p, 0, 0)) {
				return test(p, 0, 1) ? 'R' : 'L';
			} else {
				return test(p, 2, 3) ? 'A' : 'G';
			}
		} else {
			if (test(p, 5, 1)) {
				return test(p, 0, 0) ? 'B' : 'C';
			} else {
				return test(p, 0, 1) ? 'P' : 'Y';
			}
		}
	}
	return '?';
}

std::vector<int64_t> read_intcode(input_t in);

struct cpu_t {
	std::vector<int64_t> V;
	int64_t output = 0, *input = NULL;
	int i = 0, r = 0;

	enum { S_HLT, S_IN, S_OUT };

	cpu_t(const std::vector<int64_t> &V, size_t extra_mem = 16) : V(V) {
		this->V.resize(V.size() + extra_mem);
	}

	// Potentially unsafe memory access, use only with official inputs
	int run() {
		for (;;) {
			auto &a = V[i + 1], &b = V[i + 2], &c = V[i + 3];
			switch (V[i]) {
			    case     1: V[  c] = V[  a] + V[  b];    i += 4; break;
			    case   101: V[  c] =     a  + V[  b];    i += 4; break;
			    case   201: V[  c] = V[r+a] + V[  b];    i += 4; break;
			    case  1001: V[  c] = V[  a] +     b ;    i += 4; break;
			    case  1101: V[  c] =     a  +     b ;    i += 4; break;
			    case  1201: V[  c] = V[r+a] +     b ;    i += 4; break;
			    case  2001: V[  c] = V[  a] + V[r+b];    i += 4; break;
			    case  2101: V[  c] =     a  + V[r+b];    i += 4; break;
			    case  2201: V[  c] = V[r+a] + V[r+b];    i += 4; break;
			    case 20001: V[r+c] = V[  a] + V[  b];    i += 4; break;
			    case 21001: V[r+c] = V[  a] +     b ;    i += 4; break;
			    case 20101: V[r+c] =     a  + V[  b];    i += 4; break;
			    case 21101: V[r+c] =     a  +     b ;    i += 4; break;
			    case 21201: V[r+c] = V[r+a] +     b ;    i += 4; break;
			    case 22001: V[r+c] = V[  a] + V[r+b];    i += 4; break;
			    case 22101: V[r+c] =     a  + V[r+b];    i += 4; break;
			    case 22201: V[r+c] = V[r+a] + V[r+b];    i += 4; break;
			    case     2: V[  c] = V[  a] * V[  b];    i += 4; break;
			    case   102: V[  c] =     a  * V[  b];    i += 4; break;
			    case  1002: V[  c] = V[  a] *     b ;    i += 4; break;
			    case  1102: V[  c] =     a  *     b ;    i += 4; break;
			    case  1202: V[  c] = V[r+a] *     b ;    i += 4; break;
			    case  2102: V[  c] =     a  * V[r+b];    i += 4; break;
			    case  2202: V[  c] = V[r+a] * V[r+b];    i += 4; break;
			    case 20002: V[r+c] = V[  a] * V[  b];    i += 4; break;
			    case 20102: V[r+c] =     a  * V[  b];    i += 4; break;
			    case 21002: V[r+c] = V[  a] *     b ;    i += 4; break;
			    case 21102: V[r+c] =     a  *     b ;    i += 4; break;
			    case 21202: V[r+c] = V[r+a] *     b ;    i += 4; break;
			    case 22002: V[r+c] = V[  a] * V[r+b];    i += 4; break;
			    case 22102: V[r+c] =     a  * V[r+b];    i += 4; break;
			    case 22202: V[r+c] = V[r+a] * V[r+b];    i += 4; break;
			    case     3: input = &V[  a];             i += 2; return S_IN;
			    case   203: input = &V[r+a];             i += 2; return S_IN;
			    case     4: output = V[  a];             i += 2; return S_OUT;
			    case   104: output =     a ;             i += 2; return S_OUT;
			    case   204: output = V[r+a];             i += 2; return S_OUT;
			    case   105: i =     a  ? V[  b] : i + 3;         break;
			    case  1005: i = V[  a] ?     b  : i + 3;         break;
			    case  1105: i =     a  ?     b  : i + 3;         break;
			    case  1205: i = V[r+a] ?     b  : i + 3;         break;
			    case  2105: i =     a  ? V[r+b] : i + 3;         break;
			    case   106: i =     a  ? i + 3 : V[  b];         break;
			    case  1006: i = V[  a] ? i + 3 :     b ;         break;
			    case  1106: i =     a  ? i + 3 :     b ;         break;
			    case  1206: i = V[r+a] ? i + 3 :     b ;         break;
			    case  2106: i =     a  ? i + 3 : V[r+b];         break;
			    case     7: V[  c] = (V[  a] <  V[  b]); i += 4; break;
			    case   107: V[  c] = (    a  <  V[  b]); i += 4; break;
			    case  1007: V[  c] = (V[  a] <      b ); i += 4; break;
			    case  1107: V[  c] = (    a  <      b ); i += 4; break;
			    case  1207: V[  c] = (V[r+a] <      b ); i += 4; break;
			    case  2107: V[  c] = (    a  <  V[r+b]); i += 4; break;
			    case  2207: V[  c] = (V[r+a] <  V[r+b]); i += 4; break;
			    case 20107: V[r+c] = (    a  <  V[  b]); i += 4; break;
			    case 20207: V[r+c] = (V[r+a] <  V[  b]); i += 4; break;
			    case 21007: V[r+c] = (V[  a] <      b ); i += 4; break;
			    case 21107: V[r+c] = (    a  <      b ); i += 4; break;
			    case 21207: V[r+c] = (V[r+a] <      b ); i += 4; break;
			    case 22007: V[r+c] = (V[  a] <  V[r+b]); i += 4; break;
			    case 22107: V[r+c] = (    a  <  V[r+b]); i += 4; break;
			    case 22207: V[r+c] = (V[r+a] <  V[r+b]); i += 4; break;
			    case     8: V[  c] = (V[  a] == V[  b]); i += 4; break;
			    case   108: V[  c] = (    a  == V[  b]); i += 4; break;
			    case   208: V[  c] = (V[r+a] == V[  b]); i += 4; break;
			    case  1008: V[  c] = (V[  a] ==     b ); i += 4; break;
			    case  1108: V[  c] = (    a  ==     b ); i += 4; break;
			    case  1208: V[  c] = (V[r+a] ==     b ); i += 4; break;
			    case  2108: V[  c] = (    a  == V[r+b]); i += 4; break;
			    case  2208: V[  c] = (V[r+a] == V[r+b]); i += 4; break;
			    case 20008: V[r+c] = (V[  a] == V[  b]); i += 4; break;
			    case 20208: V[r+c] = (V[r+a] == V[  b]); i += 4; break;
			    case 21008: V[r+c] = (V[  a] ==     b ); i += 4; break;
			    case 21108: V[r+c] = (    a  ==     b ); i += 4; break;
			    case 21208: V[r+c] = (V[r+a] ==     b ); i += 4; break;
			    case 22208: V[r+c] = (V[r+a] == V[r+b]); i += 4; break;
			    case     9: r += V[  a];                 i += 2; break;
			    case   109: r +=     a ;                 i += 2; break;
			    case   209: r += V[r+a];                 i += 2; break;
			    case    99: i = r = 0;                           return S_HLT;
			    default: printf("Unimplemented: %ld\n", V[i]); abort();
			}
		}
	}
};

struct advent_t {
	output_t (*fn)(input_t);
};

output_t day01(input_t);
output_t day02(input_t);
output_t day03(input_t);
output_t day04(input_t);
output_t day05(input_t);
output_t day06(input_t);
output_t day07(input_t);
output_t day08(input_t);
output_t day09(input_t);
output_t day10(input_t);
output_t day11(input_t);
output_t day12(input_t);
output_t day13(input_t);
output_t day14(input_t);
output_t day15(input_t);
output_t day16(input_t);
output_t day17(input_t);
output_t day18(input_t);
output_t day19(input_t);
output_t day20(input_t);
output_t day21(input_t);
output_t day22(input_t);
output_t day23(input_t);
output_t day24(input_t);
output_t day25(input_t);

#endif
