#include "advent2019.h"

// Day 23: Category Six

constexpr int64_t N = 50;

namespace {

struct msg_t {
	int addr;
	int64_t x, y;

	msg_t() : addr(), x(), y() { }

	msg_t(int addr, int64_t x, int64_t y) :
		addr(addr), x(x), y(y)
	{
	}
};

}

output_t day23(input_t in) {
	auto V = read_intcode(in);

	std::vector<cpu_t> C(N, V);
	std::vector<msg_t> Q;
	std::vector<int64_t> NAT = { -1 };
	msg_t nat;

	// Initialize
	for (int i = 0; i < N; i++) {
		auto &c = C[i];
		c.run();
		*c.input = i;
		c.run();
	}

	// No tricks, just run the programs and pass messages
	for (int q = 0; NAT.back() != nat.y; ) {
		NAT.push_back(nat.y);
		for (int i = 0; i < N; i++) {
			auto &c = C[i];
			if (i == 0 && q) {
				*c.input = nat.x;
				c.run();
				*c.input = nat.y;
			} else {
				*c.input = -1;
			}
			for (int s = c.run(); s == cpu_t::S_OUT; s = c.run()) {
				auto addr = c.output;
				c.run(); auto x = c.output;
				c.run(); auto y = c.output;
				Q.emplace_back(addr, x, y);
			}
		}

		for (; q < Q.size(); q++) {
			auto &m = Q[q];
			auto &c = C[m.addr];
			*c.input = m.x;
			if (c.run() != cpu_t::S_IN) abort();
			*c.input = m.y;
			for (int s = c.run(); s == cpu_t::S_OUT; s = c.run()) {
				auto addr = c.output;
				if (c.run() != cpu_t::S_OUT) abort();
				auto x = c.output;
				if (c.run() != cpu_t::S_OUT) abort();
				auto y = c.output;
				if (addr == 255) {
					nat = msg_t{0,x,y};
				} else {
					Q.emplace_back(addr, x, y);
				}
			}
		}
	}

	return { NAT[2], nat.y };
}
