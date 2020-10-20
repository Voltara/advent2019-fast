#include "advent2019.h"

// Day 18: Many-Worlds Interpretation

/* Use the maze's tree structure to simplify the problem as much
 * as possible, then solve using heuristic best-first search.
 * Part 1 and Part 2 have different admissible tree reductions,
 * and are optimized separately.
 */

using mask_t = uint32_t;

constexpr int N_KEYS = 26;
constexpr int HASH_SIZE = 15000;

static void assert(bool predicate, const std::string &msg) {
	if (predicate) return;
	fprintf(stderr, "%s\n", msg.c_str());
	abort();
}

namespace {

struct node;
using tree4 = std::array<node *, 4>;

// Disjoint-set structure of keys
struct keymap {
	std::vector<int> M;
	keymap() : M(N_KEYS) {
		for (int i = 0; i < N_KEYS; i++) M[i] = i;
	}
	int find(int n) {
		while (M[n] != n) n = M[n] = M[M[n]];
		return n;
	}
	int join(mask_t key) {
		int r = find(*bits(key));
		for (auto k : ++bits(key)) M[k] = r;
		return r;
	}
};

struct node {
	std::vector<node *> C = {}; // children
	int cost = 0, max_depth = 0;
	mask_t key = 0, door = 0;
	mask_t subkey = 0, subdoor = 0;

	~node() { for (auto n : C) delete n; }

	node * clone() const {
		node *N = new node(*this);
		for (auto &n : N->C) n = n->clone();
		return N;
	}

	void rebuild_recursive_stats() {
		subkey = key;
		subdoor = door;
		max_depth = cost;
		for (auto n : C) {
			n->rebuild_recursive_stats();
			subkey |= n->subkey;
			subdoor |= n->subdoor;
			max_depth = std::max(max_depth, cost + n->max_depth);
		}
	}

	// Collapse subtree down to a single node
	int flatten(keymap &KM) {
		int base_cost = 0;
		for (auto n : C) {
			base_cost += n->flatten(KM);
			cost += n->cost;
			key |= n->key;
			door |= n->door;
			delete n;
		}
		C.clear();
		subkey = key = 1 << KM.join(key);
		base_cost += (cost - max_depth) * 2;
		cost = max_depth;
		return base_cost;
	}

	// Doors must be renumbered whenever keys might have been joined
	mask_t renumber_doors(keymap &KM) {
		mask_t tmp = door;
		door = 0;
		for (auto k : bits(tmp)) {
			door |= 1 << KM.find(k);
		}
		subdoor = door;
		for (auto n : C) subdoor |= n->renumber_doors(KM);
		return subdoor;
	}
};

struct loader {
	using grid_t = std::vector<std::string>;

	// convert a maze quadrant into a tree structure
	static void dfs(decltype(node::C) &C0, grid_t &G, int x, int y) {
		auto &c = G[x][y];
		if (c == '#') return;
		mask_t key  = (c >= 'a' && c <= 'z') ? 1 << (c - 'a') : 0;
		mask_t door = (c >= 'A' && c <= 'Z') ? 1 << (c - 'A') : 0;
		c = '#'; // prevent backtracking

		decltype(node::C) C;
		dfs(C, G, x-1, y);
		dfs(C, G, x+1, y);
		dfs(C, G, x, y-1);
		dfs(C, G, x, y+1);

		node *N;
		if (key || C.size() > 1) { // key or intersection
			(N = new node)->C = C;
		} else if (C.empty()) { // dead end
			return;
		} else { // corridor
			N = C[0];
		}

		N->cost++;
		N->key |= key;
		N->door |= door;

		C0.push_back(N);
	}

	static node * dfs(grid_t &G, int x, int y) {
		decltype(node::C) C;
		dfs(C, G, x, y);
		if (C.empty()) return new node;
		C[0]->cost--;
		C[0]->rebuild_recursive_stats();
		return C[0];
	}

	static tree4 load(input_t in) {
		grid_t G;

		for (auto end = in.s + in.len; in.s < end; ) {
			char *eol = in.s + strcspn(in.s, "\r\n");
			G.emplace_back(in.s, eol - in.s);
			in.s = eol + strspn(eol, "\r\n");
		}

		// Expected coordinate of '@' in center of maze
		int at = G.size() / 2;

		// Validate & fixup maze to ensure memory safety
		assert(G.size() >= 5, "maze too small");
		assert(G.size() % 4 == 1, "bad maze dimension");
		for (auto &s : G) {
			assert(s.size() == G.size(), "maze not square");
			s.front() = s.back() = '#';
		}
		assert(G[at][at] == '@', "'@' not in center of maze");

		// Split the maze into (acyclic for all inputs) quadrants
		G[at-1][at] = G[at+1][at] = G[at][at-1] = G[at][at+1] = '#';

		// Return quadrants in clockwise order; two quadrants
		// i, j are diagonally opposite iff (i ^ j == 2)
		return {
			dfs(G, at-1, at-1),
			dfs(G, at+1, at-1),
			dfs(G, at+1, at+1),
			dfs(G, at-1, at+1) };
	}
};

// keys collected so far, four robot positions
struct state {
	mask_t key = 0;
	uint8_t pos[4] = { N_KEYS, N_KEYS, N_KEYS, N_KEYS };

	operator uint64_t () const { return *(const uint64_t *)(this); }
	state(mask_t key) : key(key) { }
	struct hash {
		std::size_t operator() (const state &S) const {
			return std::hash<uint64_t>{}(S);
		}
	};
};

// cost, forbidden moves
struct metric {
	int cost = 0;
	mask_t exclude = 0;

	metric() { }
	metric(int cost, mask_t exclude = 0) :
		cost(cost), exclude(exclude)
	{
	}
};

using state_map_t = std::unordered_map<state, metric, state::hash>;

// priority queue entry
struct qentry {
	state_map_t::iterator state;
	int heuristic;
	qentry(decltype(state) state, int heuristic) : state(state), heuristic(heuristic) { }
	bool operator < (const qentry &o) const {
		return heuristic > o.heuristic;
	}
};

struct solver_base {
	tree4 R;

	// Distance between keys (index N_KEYS is the start position)
	int Dist[N_KEYS + 1][N_KEYS + 1] = { };

	// Which doors are in front of a key
	mask_t Door[N_KEYS] = { };

	// Heuristic minimum cost to collect each key
	int MinDist[N_KEYS] = { };

	// Goal state
	mask_t goal = 0;

	solver_base(const decltype(R) &R) : R(R) {
		for (auto n : R) {
			door_index(n);
			pairwise_distance(n);
			goal |= n->subkey;
		}
	}

	// Populate Door[] array, indexed by key number
	void door_index(node *N, mask_t door = 0) {
		door |= N->door;
		if (N->key) Door[*bits(N->key)] = door;
		door |= N->key; // ensure parent key is collected first
		for (auto n : N->C) door_index(n, door);
	}

	// Tabulate pairwise distances between keys in separate subtrees of N
	void pairwise_distance(node *N, int cost = 0) {
		cost += N->cost;
		for (auto n : N->C) {
			pairwise_distance(n, cost);
		}

		// Distance from this quadrant's starting position
		if (N->key) Dist[N_KEYS][*bits(N->key)] = cost;

		int common = cost * 2;
		for (auto n : N->C) {
			for (auto k0 : bits(n->subkey)) {
				for (auto k1 : bits(N->subkey ^ n->subkey)) {
					Dist[k0][k1] = Dist[k1][k0] = Dist[N_KEYS][k0] + Dist[N_KEYS][k1] - common;
				}
			}
		}
	}

	// Find distances between keys across quadrants
	void cross_link(mask_t key0, mask_t key1, int dist) {
		for (auto k0 : bits(key0)) {
			for (auto k1 : bits(key1)) {
				Dist[k0][k1] = Dist[k1][k0] = Dist[N_KEYS][k0] + Dist[N_KEYS][k1] + dist;
			}
		}
	}

	void cross_link() {
		// Adjacent quadrants
		cross_link(R[0]->subkey, R[1]->subkey, 2);
		cross_link(R[1]->subkey, R[2]->subkey, 2);
		cross_link(R[2]->subkey, R[3]->subkey, 2);
		cross_link(R[3]->subkey, R[0]->subkey, 2);

		// Diagonal quadrants
		cross_link(R[0]->subkey, R[2]->subkey, 4);
		cross_link(R[1]->subkey, R[3]->subkey, 4);
	}

	void compute_heuristic(mask_t key) {
		for (auto k1 : bits(key)) {
			// Distance from start position in quadrant
			MinDist[k1] = Dist[N_KEYS][k1];

			for (auto k0 : bits(key ^ (1 << k1))) {
				// Ignore moves contrary to known precedence
				if (Door[k0] & (1 << k1)) continue;
				MinDist[k1] = std::min(MinDist[k1], Dist[k0][k1]);
			}
		}
	}
};

template<int PART>
struct solver : solver_base {
	solver(const decltype(R) &R) : solver_base(R) {
		if (PART == 1) {
			// Robot can travel between quadrants
			cross_link();
			compute_heuristic(goal);
		} else {
			// Robots stay within a single quadrant
			for (auto n : R) {
				compute_heuristic(n->subkey);
			}
		}
	}

	int operator() () {
		const int r_max = (PART == 1) ? 1 : R.size();

		std::unordered_map<state, metric, state::hash> Frontier(HASH_SIZE);
		std::priority_queue<qentry> Q;

		Frontier.emplace(goal, metric{});
		Q.emplace(Frontier.begin(), 0);
		while (!Q.empty()) {
			auto q = Q.top();
			Q.pop();

			auto& [ S0, M0 ] = *q.state;

			int cost = std::exchange(M0.cost, -1);
			mask_t exclude = M0.exclude;

			if (!S0.key) {
				// solved
				return cost;
			} else if (cost == -1) {
				// already visited
				continue;
			}

			for (int r = 0; r < r_max; r++) {
				auto n = R[r];

				mask_t todo = S0.key & (n->subkey | -(PART == 1));

				for (auto k : bits(todo & ~exclude)) {
					if (Door[k] & S0.key) continue;

					auto c = cost + Dist[S0.pos[r]][k];

					// If we skip all available moves, don't try again
					// until this robot has moved elsewhere first
					if (PART == 2) exclude |= 1 << k;

					state S = S0;
					S.key ^= 1 << k;
					S.pos[r] = k;

					metric M{c, exclude & ~n->subkey};

					auto [ it, ok ] = Frontier.emplace(S, M);
					if (!ok) {
						if (c >= it->second.cost) continue;
						it->second = M;
					}
					Q.emplace(it, (q.heuristic - MinDist[k]) + (c - cost));
				}

				// Skipped all moves, but no keys left to uncover
				if (todo && !(todo & ~exclude)) {
					break;
				}
			}
		}

		return -1;
	}
};

struct optimizer {
	tree4 R;
	mask_t all_doors = 0;

	optimizer(tree4 R) : R(R) { }

	optimizer clone() const {
		optimizer O = *this;
		for (auto &n : O.R) n = n->clone();
		return O;
	}

	~optimizer() {
		for (auto n : R) delete(n);
	}

	// Optimizations valid for both parts
	int common() {
		int cost = 0;
		all_doors = 0;
		for (auto n : R) {
			remove_free_doors(n);
			all_doors |= n->subdoor;
		}
		for (auto n : R) {
			remove_unused_keys(n, all_doors);
			cost += merge_free_leaves(n, all_doors);
		}
		renumber_doors();

		return cost;
	}

	// Optimizations valid for Part 1 only: if a key is required to
	// unlock a door, it cannot be the final key collected
	int part1_only() {
		int cost = 0;
		for (auto n : R) cost += merge_free_door_keys(n, all_doors);
		renumber_doors();
		for (auto n : R) cost += flatten_free_branches(n);
		renumber_doors();
		return cost;
	}

	// Optimizations valid for Part 2 only: robots do not
	// backtrack out of their quadrant
	int part2_only() {
		int cost = 0;
		for (auto n : R) cost += merge_free_branches(n);
		renumber_doors();
		return cost;
	}

    private:
	keymap KM;

	// Common: If a door is a child of its own key, both can be ignored
	mask_t remove_free_doors(node *N, mask_t key = 0) {
		mask_t removed = N->door & key;
		for (auto n : N->C) removed |= remove_free_doors(n, key | N->key);
		N->key &= ~removed;
		N->subkey &= ~removed;
		N->door &= ~removed;
		N->subdoor &= ~removed;
		return removed;
	}

	// Common: Keys in internal nodes can be ignored if the door
	// does not need to be traversed
	void remove_unused_keys(node *N, mask_t all_doors) {
		if (N->C.empty()) return;
		N->key &= all_doors;
		N->subkey = N->key;
		for (auto n : N->C) {
			remove_unused_keys(n, all_doors);
			N->subkey |= n->subkey;
		}
	}

	// Common: Children may be merged if they meet the following:
	//     - no doors
	//     - the keys don't open any doors
	int merge_free_leaves(node *N, mask_t all_doors) {
		int cost = 0;

		for (auto n : N->C) {
			cost += merge_free_leaves(n, all_doors);
		}

		auto it = N->C.begin();
		node *t = NULL;

		for (auto n : N->C) {
			if ((n->subkey & all_doors) || n->subdoor) {
				*it++ = n;
			} else if (!t) {
				t = n;
			} else {
				t->key |= n->key;
				t->max_depth = std::max(t->max_depth, n->max_depth);
				t->cost += n->cost;
				delete n;
			}
		}

		if (t) {
			cost += (t->cost - t->max_depth) * 2;
			t->cost = t->max_depth;
			if (t->key) t->key = 1 << KM.join(t->key);
			*it++ = t;
		}

		N->C.erase(it, N->C.end());

		// Merge upward (a door is allowed on this node)
		if (N->C.size() == 1 && !(N->subkey & all_doors)) {
			auto n = N->C[0];
			if (!n->subdoor) {
				N->C.pop_back();
				if (n->key) N->key = 1 << KM.join(N->key | n->key);
				N->cost += n->cost;
				delete n;
			}
		}

		N->subkey = N->key;
		for (auto n : N->C) N->subkey |= n->subkey;

		return cost;
	}

	// Part 1: Branches with no doors can be taken all-or-nothing
	int flatten_free_branches(node *N) {
		int cost = 0;
		N->subkey = N->key;
		for (auto n : N->C) {
			if (!n->subdoor) {
				cost += n->flatten(KM);
			} else {
				cost += flatten_free_branches(n);
			}
			N->subkey |= n->subkey;
		}
		return cost;
	}

	// Part 1: Keys required for opening doors must backtrack,
	// so they can be taken greedily
	int merge_free_door_keys(node *N, mask_t all_doors) {
		int cost = 0;

		for (auto n : N->C) {
			cost += merge_free_door_keys(n, all_doors);
		}

		auto it = N->C.begin();
		for (auto n : N->C) {
			if (n->subdoor || (n->subkey & ~all_doors)) {
				*it++ = n;
			} else {
				N->key |= n->key;
				cost += n->cost * 2;
				delete n;
			}
		}
		N->C.erase(it, N->C.end());

		if (N->key) N->key = 1 << KM.join(N->key);

		N->subkey = N->key;
		for (auto n : N->C) N->subkey |= n->subkey;

		return cost;
	}

	// Part 2: Greedily take branches satisfying the following:
	//     - no doors
	//     - no backtracking possible
	//     - not the deepest branch among siblings
	int merge_free_branches(node *N) {
		int max_depth = N->max_depth - N->cost;
		int cost = 0;

		auto it = N->C.begin();
		for (auto n : N->C) {
			if ((n->max_depth < max_depth) && !n->subdoor) {
				cost += n->flatten(KM);
				cost += n->cost * 2;
				N->key |= n->key;
				delete n;
			} else {
				*it++ = n;
			}
		}
		N->C.erase(it, N->C.end());

		// Continue search only if one child (no backtracking)
		if (N->C.size() == 1) {
			cost += merge_free_branches(N->C[0]);
		}

		if (N->key) N->key = 1 << KM.join(N->key);

		N->subkey = N->key;
		for (auto n : N->C) N->subkey |= n->subkey;

		return cost;
	}

	void renumber_doors() {
		all_doors = 0;
		for (auto n : R) all_doors |= n->renumber_doors(KM);
	}
};

}

output_t day18(input_t in) {
	optimizer O1{loader::load(in)};

	int part1 = O1.common(), part2 = part1;

	auto O2 = O1.clone();

	part1 += O1.part1_only() + 2;
	part1 += solver<1>{O1.R}();

	part2 += O2.part2_only();
	part2 += solver<2>{O2.R}();

	return { part1, part2 };
}
