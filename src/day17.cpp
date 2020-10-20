#include "advent2019.h"

// Day 17: Set and Forget

constexpr int LIMIT = 20;
constexpr int MLIMIT = 10;

namespace {

struct pt {
	int x, y;
	pt() : x(), y() { }
	pt(int x, int y) : x(x), y(y) { }
};

struct sol_t {
	std::vector<int> M;
	std::vector<int> P[3];
	sol_t() : M(), P() { }
};

}

static bool solve(sol_t &S, const std::vector<int8_t> &P, int idx, int used) {
	if (idx == P.size()) return true;
	if (S.M.size() == MLIMIT) return false;

	// Recycle existing program
	for (int i = 0; i < used; i++) {
		auto &prog = S.P[i];
		if (idx + prog.size() > P.size()) continue;

		bool nope = false;
		for (int j = 0; j < prog.size(); j++) {
			if (P[idx + j] != prog[j]) {
				nope = true;
				break;
			}
		}
		if (nope) continue;

		S.M.push_back(i);
		if (solve(S, P, idx + prog.size(), used)) {
			return true;
		}
		S.M.pop_back();
	}

	if (used == 3) return false;

	// Write new program
	auto &prog = S.P[used];
	S.M.push_back(used);
	for (int cost = -1; idx < P.size(); idx++) {
		cost += 4 + ((P[idx] & 0x7f) > 9);
		if (cost > LIMIT) break;
		prog.push_back(P[idx]);
		if (solve(S, P, idx + 1, used + 1)) {
			return true;
		}
	}
	prog.clear();
	S.M.pop_back();

	return false;
}

output_t day17(input_t in) {
	// The Intcode program needs extra memory
	constexpr int EXTRA_MEM = 8192;

	auto V = read_intcode(in);

	constexpr int DIM = 100;
	char G[DIM][DIM] = { };

	pt robot;

	int part1 = 0, maxx = 0, run = 0;

	cpu_t C1(V, EXTRA_MEM);
	for (pt p{1,1}; C1.run() != cpu_t::S_HLT; p.x++) {
		if (p.x < 1 || p.x >= DIM) abort();
		if (p.y < 1 || p.y >= DIM) abort();
		switch (C1.output) {
		    case '^': robot = p;
		    case '#': G[p.x][p.y] = 1;
			      maxx = std::max(maxx, p.x + 1);
			      break;
		    case '\n': p.x = 0, p.y++;
		}
		// Check for intersection
		run = (run + (C1.output == '#')) & -(C1.output == '#');
		if (run >= 3 && G[p.x - 1][p.y - 1]) {
			part1 += (p.x - 2) * (p.y - 1);
		}
	}

	// Split the path into segments by greedily moving as
	// far as possible before turning
	std::vector<int8_t> I;
	int dx = 0, dy = -1, steps = 0;
	for (pt p = robot; ; p = { p.x + dx, p.y + dy }) {
		if (G[p.x + dx][p.y + dy]) {
			// ahead
			steps++;
		} else if (G[p.x + dy][p.y - dx]) {
			// left
			if (steps) I.back() |= steps;
			I.push_back(0x80);
			dx = std::exchange(dy, -dx);
			steps = 1;
		} else if (G[p.x - dy][p.y + dx]) {
			// right
			if (steps) I.back() |= steps;
			I.push_back(0x00);
			dy = std::exchange(dx, -dy);
			steps = 1;
		} else {
			// goal
			break;
		}
	}
	I.back() |= steps;

	// Find a program that solves the maze
	sol_t S;
	if (!solve(S, I, 0, 0)) abort();

	// Convert the program to text
	std::string s;
	for (int i = 0; i < S.M.size(); i++) {
		if (i) s.push_back(',');
		s.push_back(S.M[i] + 'A');
	}
	s.push_back('\n');
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < S.P[i].size(); j++) {
			if (j) s.push_back(',');
			int k = S.P[i][j];
			s.push_back((k & 0x80) ? 'L' : 'R');
			k &= 0x7f;
			s.push_back(',');
			if (k > 9) {
				s.push_back('1');
				k -= 10;
			}
			s.push_back(k + '0');
		}
		s.push_back('\n');
	}
	s.push_back('n');
	s.push_back('\n');

	// Run the program and collect the Part 2 solution
	V[0] = 2;
	cpu_t C2(V, EXTRA_MEM);
	for (auto c : s) {
		while (C2.run() != cpu_t::S_IN) { }
		*C2.input = c;
	}
	int part2 = 0;
	while (C2.run() == cpu_t::S_OUT) {
		part2 = C2.output;
	}

	return { part1, part2 };
}
