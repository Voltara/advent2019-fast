# advent2019-fast

[Advent of Code 2019](https://adventofcode.com/2019/) optimized C++ solutions.

Here are the timings from an example run on an i9-9980HK CPU laptop.

    Day 01       4 μs
    Day 02       4 μs
    Day 03      41 μs
    Day 04       7 μs
    Day 05       7 μs
    Day 06      84 μs
    Day 07       8 μs
    Day 08      14 μs
    Day 09     913 μs
    Day 10     248 μs
    Day 11     220 μs
    Day 12     765 μs
    Day 13   1,379 μs
    Day 14     136 μs
    Day 15     164 μs
    Day 16     345 μs
    Day 17     468 μs
    Day 18     346 μs
    Day 19      45 μs
    Day 20     203 μs
    Day 21   1,466 μs
    Day 22      11 μs
    Day 23     607 μs
    Day 24     233 μs
    Day 25   1,195 μs
    -----------------
    Total:   8,913 μs

Solutions should work with any puzzle input, provided it is byte-for-byte an exact copy of the file downloaded from Advent of Code.  Be careful when using unofficial inputs, because the Intcode implementation does not enforce memory safety.

This code makes use of SIMD instructions (Day 12 only), and requires an x86 CPU that supports the SSSE3 instruction set.

# Summary of solutions

Here are a few brief notes about each solution.

## Day 1

Straightforward computation; not much to optimize here.  If the input were significantly longer, SIMD may have had some benefit.

## Day 2

A polynomial disguised as Intcode.  In this case, a linear system in two unknowns, which we can solve after running the program twice.  To avoid unnecessary overhead, I use a specialized Intcode implementation.

## Day 3

Each wire is represented as a list of segments.  A factor of two is saved by comparing only perpendicular pairs of wires.

## Day 4

Digit-by-digit dynamic programming.  There are an equivalent number of valid ways to complete the prefixes `113???` and `223???`, so (aside from accounting for the lower/upper bounds) we can reuse the result.

## Day 5

Straightforward run-the-Intcode problem, again using a specialized implementation.

## Day 6

Dynamic programming for Part 1, and lowest common ancestor for Part 2.

## Day 7

In Part 1, the amplifier phases apply a linear function (`ax + b`) with small coefficients.  Because the coefficients are small, we can find both by testing once with a large enough value of `x`.  The Part 2 amplifiers each apply some sequence of ten `+1` or `*2` operations which can be identified by running each ten times with an input of `0`.

With each amplifier's function known, optimal outputs are found using dynamic programming on subsets.

## Day 8

The Part 2 output text is recognized by a decision tree that examines at most 4 pixels per character.  Although it recognizes only 15 letters, puzzle solutions only use a subset of the alphabet.

## Day 9

The first solution use the fully-featured Intcode implementation, which is reused by all later Intcode problems.

## Day 10

Uses a lookup table of reduced fractions to simplify the visibility checks.  The Part 2 answer always seems to be in the first layer of the third quadrant, so this solution looks there.

A more general, but still fast Part 2 solution could index the rays by number of asteroids intersected.  This would make it possible to quickly skip to the layer where the target asteroid is found.

## Day 11

Part 1 is a straightforward implementation using 2 bits of storage per location (color, visited) in a fixed grid.  Part 2 reuses the character recognition from Day 8.

A possible further optimization to Part 2 would be to terminate the Intcode as soon as the enough pixels are drawn to recognize the characters.  Most time is spent in Part 1, though.

## Day 12

Computes all four bodies in parallel using SIMD.  For Part 2, it is only necessary to step through half of the cycle (the velocities will become zero again), then double the count.

## Day 13

With so many Intcode puzzles this year, I had to decide what optimization techniques I would consider "fair game".  I decided that examining program data directly would be forbidden, but any insight that could be gained from *running* the program and observing its output is allowed.  This led to what I felt were more interesting solutions.

Because the Day 13 solution depends on an opaque scoring algorithm, I opted to "just play the game" and rely on my Intcode implementation for speed.

## Day 14

Part 1 is dynamic programming, and Part 2 is interpolation search over the Part 1 function.

## Day 15

Solves both parts in a single depth-first search through the maze.  Because it does not need to save a copy of the map, memory usage is proportional to the greatest distance from the starting position.

Speed can be improved by remembering the locations of walls, which might be tested twice, once from each side.  I like the simplicity of the memoryless implementation, though.

## Day 16

Part 1 uses a prefix sum array, and Part 2 solves digit-by-digit using binomial coefficients.  Rather than attempt to describe the technique, I will link to [this post](https://www.reddit.com/r/adventofcode/comments/ebqgdu/2019_day_16_part_2_lets_combinatorics/) in the subreddit.

Reading `C(n, k)` as "n choose k", further optimization comes from the simplicity of `C(k+99, k)` modulo 2 and 5.  Specifically:

    C(k+99, k) % 5 = 1 ; if k % 125 = 0
                   = 4 ; if k % 125 = 25
                   = 0 ; otherwise
    
    C(k+99, k) % 2 = 1 ; if k % 128 = { 0, 4, 8, 12, 16, 20, 24, 28 }
                   = 0 ; otherwise

Because of this and the periodic nature of the expanded input string, most of the digits can actually be ignored.

## Day 17

Recursive backtracking search for a program that fits within the memory constraints.

## Day 18

By far the most complicated solution of the set, which is ultimately a heuristic best-first search with tree reduction pre-pass.  The maze is structured as four trees rooted at a central cyclic hub.

Before running the search, it first tries to reduce the number of keys (and therefore number of nodes expanded) as much as possible.  For example, if a key is directly on the path to its door from the start position, both the key and its door can be ignored.

Another more subtle optimization (valid for Part 1 only) is, if a branch of the maze has no doors blocking its keys, those keys should be collected all-or-nothing because it is always suboptimal to visit that branch multiple times.

## Day 19

Minimizes the number of Intcode invocations by modeling the tractor beam as the region bounded by two rays.  The rays can have irrational slope, the code finds a close rational approximation by Stern-Brocot tree search.

Part 1 uses a fast Euclidean-like algorithm for counting lattice points beneath a line.  Part 2 uses the slopes to calculate the location of the nearest 99x99 opening.

## Day 20

Locates the outer portals by stepping around the perimeter looking for openings.  Once the outer portals are found, searches all connected components of the maze to find pairwise distances between portals.  Finally, it finds the shortest path with best-first search (Part 2 is a bidirectional search.)

## Day 21

Uses manually written Springdroid programs capable of solving any input.

## Day 22

Fast composition of a modular linear function using exponentiation by squaring.

## Day 23

A straightforward solution for reasons similar to Day 13.

## Day 24

The only bit twiddling solution this year.  Represents each 5x5 grid as a 64-bit integer, using 2 bits for each cell.  Uses SWAR (SIMD within a register) techniques to quickly count neighbors in parallel.

## Day 25

Each of the eight items has a different power-of-two weight.  This makes it possible to iteratively keep or discard groups of the heaviest unclassified items.  If known items are appropriately carried or discarded, they can be ignored when classifying the remaining items.  Then, if the heaviest unknown item is not part of the solution, carrying it will exceed the weight threshold.  Conversely, if it *is* part of the solution, dropping it will bring the weight below threshold.
