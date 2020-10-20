#include <chrono>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "advent2019.h"

// Allows solutions to read past the end of the input safely
static constexpr size_t BACKSPLASH_SIZE = 1 << 20;

static const std::vector<advent_t> advent2019 = {
	{ day01 }, { day02 }, { day03 }, { day04 }, { day05 },
	{ day06 }, { day07 }, { day08 }, { day09 }, { day10 },
	{ day11 }, { day12 }, { day13 }, { day14 }, { day15 },
	{ day16 }, { day17 }, { day18 }, { day19 }, { day20 },
	{ day21 }, { day22 }, { day23 }, { day24 }, { day25 }
};

static input_t load_input(const std::string &filename);
static void free_input(input_t &input);

int main() {
	double total_time = 0;

	printf("          Time        Part 1           Part 2\n");
	printf("=======================================================\n");
	for (int day = 1; day <= advent2019.size(); day++) {
		auto &A = advent2019[day - 1];
		if (!A.fn) continue;

		char filename[64];
		sprintf(filename, "input/day%02d.txt", day);

		auto input = load_input(filename);
		auto t0 = std::chrono::steady_clock::now();
		auto output = A.fn(input);
		auto elapsed = std::chrono::steady_clock::now() - t0;
		free_input(input);

		double t = 1e-3 * std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
		total_time += t;

		printf("Day %02d: %6.f μs     %-16s %-16s\n",
				day,
				t,
				output.part1.c_str(),
				output.part2.c_str());
	}
	printf("=======================================================\n");
	printf("Total:  %6.f μs\n", total_time);

	return 0;
}

input_t load_input(const std::string &filename) {
	static void *backsplash = NULL;
	input_t input;

	backsplash = mmap(backsplash, BACKSPLASH_SIZE, PROT_READ, MAP_PRIVATE|MAP_ANONYMOUS|(backsplash ? MAP_FIXED : 0), -1, 0);
	if (backsplash == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	int fd = open(filename.c_str(), O_RDONLY);
	if (fd == -1) {
		perror(filename.c_str());
		exit(EXIT_FAILURE);
	}
	input.len = lseek(fd, 0, SEEK_END);
	if (input.len > BACKSPLASH_SIZE) {
		std::cerr << "Why is your input so big?\n";
		exit(EXIT_FAILURE);
	}
	input.s = reinterpret_cast<char *>(mmap(backsplash, input.len, PROT_READ, MAP_PRIVATE|MAP_FIXED, fd, 0));
	if (input.s == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}
	if (input.s != backsplash) {
		std::cerr << "Warning: Input not mapped at the expected location.\n";
	}
	if (close(fd) == -1) {
		perror(filename.c_str());
		exit(EXIT_FAILURE);
	}
	return input;
}

void free_input(input_t &input) {
	if (munmap(input.s, input.len) == -1) {
		perror("munmap");
		exit(EXIT_FAILURE);
	}
}
