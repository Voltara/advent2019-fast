#include "advent2019.h"

// Day 25: Cryostasis

constexpr int N_ITEMS = 8;

// Lowest 4 bits of the ASCII codes for 's', 'e', 'w' and 'n'
enum { M_SOUTH = 3, M_EAST = 5, M_WEST = 7, M_NORTH = 14 };
static int BACK[] = {
	0, 0, 0, M_NORTH, 0, M_WEST, 0, M_EAST,
	0, 0, 0, 0, 0, 0, M_SOUTH, 0 };

static const std::string SEC_ROOM = "== Security Checkpoint ==";
static const std::vector<std::string> MOVE = {
	"", "", "", "south\n", "", "east\n", "", "west\n",
	"", "", "", "", "", "",  "north\n", "" };
static const std::unordered_set<std::string> AVOID = {
	"infinite loop", "escape pod", "molten lava",
	"giant electromagnet", "photons" };

namespace {

struct room {
	std::string name;
	std::vector<std::string> inv;
	int exit;
	bool is_end;
	int64_t keycode;

	room() : name(), inv(), exit(), is_end(), keycode() { }
	void reset() {
		name.clear();
		inv.clear();
		exit = 0;
		is_end = false;
		keycode = 0;
	}
};

struct solver {
	cpu_t C;
	room R;
	std::vector<std::string> inv;
	std::vector<int> sec_path;
	int goal_room = 0;
	int status = -1;
	int8_t weights[256] = { };

	solver(input_t in) : C(read_intcode(in)) { }

	int run() {
		return (status = C.run());
	}

	void send(const std::string &s) {
		for (auto c : s) {
			while (status != cpu_t::S_IN) run();
			*C.input = c;
			status = -1;
		}
	}

	bool recv(std::string &line) {
		line.clear();
		while (run() == cpu_t::S_OUT) {
			if (C.output == '\n') {
				if (line.empty()) continue;
				break;
			}
			line.push_back(char(C.output));
		}
		return (status == cpu_t::S_HLT);
	}

	void take(const std::string &s) {
		send("take ");
		send(s);
		send("\n");
		std::string tmp;
		recv(tmp);
		recv(tmp);
	}

	void drop(const std::string &s) {
		send("drop ");
		send(s);
		send("\n");
		std::string tmp;
		recv(tmp);
		recv(tmp);
	}

	void parse_room() {
		std::string tmp;
		R.reset();
		recv(R.name);
		recv(tmp); // Room description
		recv(tmp); // Doors here lead:
		for (;;) {
			recv(tmp);
			if (tmp[0] != '-') break;
			R.exit |= 1 << (tmp[2] & 15);
		}
		if (tmp[0] == 'C') return;
		for (recv(tmp); tmp[0] != 'C'; recv(tmp)) {
			R.inv.push_back(tmp.substr(2));
		}
	}

	// Pick up all safe items and return to the start room,
	// and remember how to get to the security checkpoint
	std::pair<int,bool> collect_items(int back = 0, int missing = N_ITEMS, bool parent_sec = false) {
		parse_room();

		// Found the security room?
		if (R.name == SEC_ROOM) {
			goal_room = *bits(R.exit ^ back);
			return { missing, true };
		}

		// Pick up items
		for (auto &item : R.inv) {
			if (AVOID.count(item)) continue;
			take(item);
			inv.push_back(item);
			missing--;
		}

		// Ready to backtrack toward the security room?
		if (!missing && parent_sec) {
			return { missing, false };
		}

		bool child_sec = false;

		// Search all branches
		for (auto d : bits(R.exit ^ back)) {
			auto b = BACK[d];

			send(MOVE[d]);

			auto [ miss, sec ] = collect_items(1 << b, missing);
			missing = miss;
			if (!missing && sec) {
				return { missing, sec };
			} else if (sec) {
				// Need to return down this path
				sec_path.push_back(d);
				child_sec = true;
			}

			send(MOVE[b]);

			// Done searching?
			if (!missing && (child_sec || parent_sec)) {
				break;
			}
		}

		return { missing, child_sec };
	}

	int attempt() {
		std::string tmp;
		send(MOVE[goal_room]);
		recv(tmp); // Pressure-sensitive floor
		recv(tmp); // Analyzing
		recv(tmp); // Doors here lead
		recv(tmp); // - direction
		recv(tmp);
		char which = tmp[59];
		if (which == 'e') {
			recv(tmp);
			recv(tmp);
			for (auto k : tmp) {
				uint8_t c = k - '0';
				if (c < 10) R.keycode = 10 * R.keycode + c;
			}
			return 0;
		}
		for (;;) {
			recv(tmp);
			if (tmp[0] == 'C') break;
		}
		if (which == 'h') return -1;
		if (which == 'l') return 1;
		abort();
		return 0;
	}

	void solve() {
		// Return to the security room
		while (!sec_path.empty()) {
			send(MOVE[sec_path.back()]);
			parse_room();
			sec_path.pop_back();
		}

		int unknown = 0xff, have = 0xff, keep = 0x00;

		// Switch up inventory
		auto choose = [&](int want) {
			for (auto i : bits(have & ~want)) drop(inv[i]);
			for (auto i : bits(~have & want)) take(inv[i]);
			have = want;
		};

		// Check weight of specified items, plus kept items
		auto check = [&](int want) {
			want |= keep;
			auto &M = weights[want];
			if (!M) {
				choose(want);
				M = attempt();
				if (M == 1) {
					// Propagate memoization by +1 item
					for (auto i : bits(0xff ^ want)) {
						weights[want | (1 << i)] = 1;
					}
				}
			}
			return M;
		};

		/* The solution always includes exactly 4 of the 8 items
		 * (there are 70 ways to choose 4 items out of 8), but this
		 * implementation does not exploit that fact.
		 *
		 * Each item has a different power-of-two weight.  As a
		 * result, each item is heavier than the combined weight
		 * of all lighter items.
		 *
		 * We split the items into three categories:
		 *   - keep: Item is part of the solution, so we keep it
		 *           in inventory, but otherwise ignore it.
		 *   - discard: Item is not part of the solution.
		 *   - unknown: Item status is not known yet.
		 *
		 * Representing the item list as a binary number, with
		 * ones indicating membership in the solution set, and
		 * more significant bits indicating heavier items.
		 * The following algorithm iteratively removes groups
		 * of leading 0-bits and 1-bits from this number.
		 */
		do {
			/* Pick up one unknown item at a time.  If our weight
			 * goes over the threshold, we can discard that item,
			 * because we know it cannot be part of the solution.
			 */
			for (auto i : bits(unknown)) {
				auto c = check(1 << i);
				if (c <= 0) {
					if (!c) return;
					continue;
				}
				unknown ^= 1 << i;
			}

			/* Pick up all unknown items, then drop one at
			 * at a time (always picking an item back up before
			 * dropping the next.)  If dropping an item causes
			 * our weight to go under the threshold, we keep
			 * the item as part of the solution.
			 */
			for (auto i : bits(unknown)) {
				auto c = check(unknown ^ (1 << i));
				if (c >= 0) {
					if (!c) return;
					continue;
				}
				unknown ^= 1 << i;
				keep |= 1 << i;
			}
		} while (check(unknown) != 0);
	}
};

}

output_t day25(input_t in) {
	solver S(in);

	S.collect_items();
	S.solve();

	return { S.R.keycode, "" };
}
