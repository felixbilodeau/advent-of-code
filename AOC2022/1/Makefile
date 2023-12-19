all: part1/main part2/main

part1/main: part1/main.ml
	ocamlopt -o part1/main part1/main.ml
	make clean-part-1

run-part-1: part1/main
	part1/main < input.txt

part2/main: part2/main.ml
	ocamlopt -o part2/main part2/main.ml
	make clean-part-2

run-part-2: part2/main
	part2/main < input.txt

clean-part-1:
	rm part1/main.o part1/main.cmx part1/main.cmi

clean-part-2:
	rm part2/main.o part2/main.cmx part2/main.cmi
