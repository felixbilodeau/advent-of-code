CC := gcc
CFLAGS := -Wall -Wextra -Werror --std=c89 -Og -ggdb

default: part1/main part2/main

part1/main: part1/main.c
	$(CC) $(CFLAGS) $^ -o $@

part2/main: part2/main.c
	$(CC) $(CFLAGS) $^ -o $@

run-part-1: part1/main
	part1/main < input.txt

run-part-2: part2/main
	part2/main < input.txt
