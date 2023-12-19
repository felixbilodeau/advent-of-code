CC := gcc
CFLAGS := -Wall -Wextra -Werror --std=c89 -O3

all: part1/main part2/main

part1/main: part1/main.c
	$(CC) $(CFLAGS) $^ -o $@

part2/main: part2/main.c
	$(CC) $(CFLAGS) $^ -o $@

run-part-1: part1/main
	cat input.txt | part1/main

run-part-2: part2/main
	cat input.txt | part2/main
