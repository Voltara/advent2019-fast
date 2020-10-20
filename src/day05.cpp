#include "advent2019.h"

// Day 5: Sunny with a Chance of Asteroids

// The last of the specialized Intcode implementations
static int64_t run(std::vector<int64_t> V, int64_t input) {
	int64_t output = 0;
	for (int i = 0; V[i] != 99; ) {
		auto &a = V[i + 1], &b = V[i + 2], &c = V[i + 3];
		switch (V[i]) {
		    case    1: V[c] = V[a] + V[b];    i += 4; break;
		    case  101: V[c] =   a  + V[b];    i += 4; break;
		    case 1001: V[c] = V[a] +   b;     i += 4; break;
		    case 1101: V[c] =   a  +   b;     i += 4; break;
		    case    2: V[c] = V[a] * V[b];    i += 4; break;
		    case  102: V[c] =   a  * V[b];    i += 4; break;
		    case 1002: V[c] = V[a] *   b;     i += 4; break;
		    case 1102: V[c] =   a  *   b;     i += 4; break;
		    case    3: V[a] = input;          i += 2; break;
		    case    4: output = V[a];         i += 2; break;
		    case  104: output =   a;          i += 2; break;
		    case  105: i =   a  ? V[b] : i + 3;       break;
		    case 1005: i = V[a] ?   b  : i + 3;       break;
		    case 1105: i =   a  ?   b  : i + 3;       break;
		    case  106: i =   a  ? i + 3 : V[b];       break;
		    case 1006: i = V[a] ? i + 3 :   b;        break;
		    case 1106: i =   a  ? i + 3 :   b;        break;
		    case    7: V[c] = (V[a] <  V[b]); i += 4; break;
		    case  107: V[c] = (  a  <  V[b]); i += 4; break;
		    case 1007: V[c] = (V[a] <    b ); i += 4; break;
		    case 1107: V[c] = (  a  <    b ); i += 4; break;
		    case    8: V[c] = (V[a] == V[b]); i += 4; break;
		    case  108: V[c] = (  a  == V[b]); i += 4; break;
		    case 1008: V[c] = (V[a] ==   b ); i += 4; break;
		    case 1108: V[c] = (  a  ==   b ); i += 4; break;
		}
	}
	return output;
}

output_t day05(input_t in) {
	auto V = read_intcode(in);
	return { run(V, 1), run(V, 5) };
}
