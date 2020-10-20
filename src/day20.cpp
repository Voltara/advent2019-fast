#include "advent2019.h"

// Day 20: Donut Maze

namespace {

// Multipurpose (y,x) pair
struct pt {
	int y, x;
	pt(int y, int x) : y(y), x(x) { }
	pt operator+ (const pt &o) const { return { y+o.y, x+o.x }; }
	pt operator- (const pt &o) const { return { y-o.y, x-o.x }; }
	pt operator- () const { return { -y, -x }; }
	pt right() const      { return {  x, -y }; }
	pt left() const       { return { -x,  y }; }
};

// Describes a move in the maze: portal id, distance, z-coordinate
struct move {
	int id, cost, z;
	move(int id, int cost, int z) : id(id), cost(cost), z(z) { }
	bool operator< (const move &o) const { return cost > o.cost; }
};

// A little memory safety in case of malformed input
struct grid {
	std::vector<std::string::iterator> Row = { };
	std::string s;

	// Need to copy the input because we modify it
	grid(input_t in) : s(in.s, in.len) {
		// Index the string by row
		Row.push_back(s.begin());
		for (auto p = s.begin(); p != s.end(); p++) {
			if (*p == '\n') Row.push_back(p + 1);
		}
		if (Row.back() != s.end()) {
			Row.push_back(s.end());
		}
	}

	char& at(int y, int x) {
		static char space = ' ';
		if (y < 0 || y + 1 >= Row.size()) return space;
		auto r = &Row[y];
		if (x < 0 || r[0] + x >= r[1]) return space;
		return r[0][x];
	}

	char& operator[] (pt p) {
		return at(p.y, p.x);
	}
};

struct solver {
	// Position on perimeter to begin looking for portals
	const pt MAZE_BORDER = pt{2,3};

	// Avoid overflowing signed char in grid
	const int MAX_LABELS = 50;

	grid G;

	// Mapping from portal label to id+1
	std::unordered_map<int,int> Label = { };
	int n_labels = 0;

	// Portals on perimeter; value is {location, direction},
	// where direction points into the maze
	std::vector<std::pair<pt,pt>> Portals = { };

	// Valid moves from each portal
	std::vector<std::vector<move>> Move = { };

	// start (AA) and goal (ZZ) portal id
	int start = 0, goal = 0;

	solver(input_t in) : G(in) {
		scan_for_portals();
		pathfind();
		start = Label[L('A','A')], goal = Label[L('Z','Z')];
		if (!start-- || !goal--) abort();
	}

	// Encode a label for unordered_map key
	static int L(char a, char b) {
		return (a << 8) | b;
	}

	// Read a label off the map and return an id for it.
	// Also records the label id on the map using characters
	// starting with lowercase 'a'.
	int scan_label(pt p, pt step) {
		char a = G[p], b = G[p + step];
		if (step.x + step.y < 0) std::swap(a, b);
		auto &l = Label[L(a,b)];
		if (!l) l = ++n_labels;
		if (n_labels > MAX_LABELS) abort();
		G[p] = 'a' + l - 1;
		return l - 1;
	}

	// Walk around the perimeter of the map looking for portals
	void scan_for_portals() {
		pt p = MAZE_BORDER, step{0,1};
		for (int turn = 0; turn < 4; p = p + step) {
			auto c = G[p];
			if (c == '.') {
				auto left = step.left();
				scan_label(p + left, left);
				Portals.emplace_back(p, step.right());
				Move.emplace_back();
			} else if (c != '#') {
				p = p - step;
				step = step.right();
				turn++;
			}
		}
	}

	// Pathfind starting from each of the portals on the perimeter;
	// if two outer portals are connected, skip the second one
	void pathfind() {
		int id = 0;
		for (auto [ p, step ] : Portals) {
			if (G[p] == '.') {
				std::vector<move> W = { { id, 0, 0 } };
				pathfind(p, step, 0, W, 0);
			}
			id++;
		}
	}

	void pathfind(pt p, pt step, int depth, std::vector<move> &W, int base) {
		char c = G[p];

		if (c >= 'a' && c - 'a' < n_labels) {
			// Outer portal
			G[p - step] = '#'; // prevent redundant search
			W.emplace_back(c - 'a', depth - 1, 0);
		} else if (c >= 'A' && c <= 'Z') {
			// Inner portal
			int id = scan_label(p, step);
			W.emplace_back(id, depth, 1);
		}

		if (c != '.') return;

		// Branch left, forward, right
		step = -step;
		for (int i = 0; i < 3; i++) {
			int pivot = W.size();
			step = step.right();
			pathfind(p + step, step, depth + 1, W, pivot);

			// Find pairwise distances between old/new portals
			for (int ia = base; ia < pivot; ia++) {
				auto &a = W[ia];
				for (int ib = pivot; ib < W.size(); ib++) {
					auto &b = W[ib];
					int cost = a.cost + b.cost - 2 * depth;
					Move[a.id].emplace_back(b.id, cost, b.z - a.z);
					Move[b.id].emplace_back(a.id, cost, a.z - b.z);
				}
			}
		}
	}

	/* We could almost solve part1 by taking the minimum distance
	 * from AA to ZZ across all depths in part2, but that search
	 * terminates too early to guarantee a correct result.
	 */
	int part1() {
		std::vector<int> D(n_labels, INT32_MAX);
		std::priority_queue<move> Q;

		D[start] = 0;
		Q.emplace(start, 0, 0);

		while (!Q.empty()) {
			auto p = Q.top();
			Q.pop();

			if (p.id == goal) break;

			for (auto m : Move[p.id]) {
				move dst{m.id, p.cost + m.cost, 0};
				if (dst.cost < D[dst.id]) {
					D[dst.id] = dst.cost;
					Q.push(dst);
				}
			}
		}

		return D[goal];
	}

	// Use bidirectional search to reduce number of nodes expanded
	int part2() {
		std::vector<std::vector<std::pair<int,int>>> D;
		std::priority_queue<move> Qa, Qz;

		auto expand_D = [&] () {
			D.emplace_back(n_labels, std::make_pair(INT32_MAX,INT32_MAX));
		};

		expand_D();
		D[0][start].first = 0;
		D[0][goal].second = 0;

		Qa.emplace(start, 0, 0);
		Qz.emplace(goal, 0, 0);

		int answer = INT32_MAX;
		int ta = 0, tz = 0; // top-of-queue costs

		while (ta + tz < answer && !(Qa.empty() && Qz.empty())) {
			if (!Qa.empty()) ta = Qa.top().cost;
			if (!Qz.empty()) tz = Qz.top().cost;

			// Keep the queues balanced
			bool which = !Qa.empty() && (Qa.size() <= Qz.size());

			auto &Q = which ? Qa : Qz;

			auto p = Q.top();
			Q.pop();

			auto [ cost_a, cost_z ] = D[p.z][p.id];
			if (p.cost != (which ? cost_a : cost_z)) continue;

			// Candidate answer
			if (cost_a != INT32_MAX && cost_z != INT32_MAX) {
				answer = std::min(answer, cost_a + cost_z);
				continue;
			}

			for (auto m : Move[p.id]) {
				move dst{m.id, p.cost + m.cost, p.z + m.z};
				if (dst.z < 0) continue;
				if (dst.z == D.size()) expand_D();

				auto &d = D[dst.z][dst.id];
				auto &cost = which ? d.first : d.second;
				if (dst.cost < cost) {
					cost = dst.cost;
					Q.push(dst);
				}
			}
		}

		return answer;
	}
};

};

output_t day20(input_t in) {
	solver S{in};

	int part1 = S.part1();
	int part2 = S.part2();

	return { part1, part2 };
}
