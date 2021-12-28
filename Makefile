# .PHONY all run

all: run_test run

test: test.c test.h array.h array.c
	gcc -Wall test.c -o test

run_test: test
	./test

main: main.c
	gcc -Wall main.c -o main

run: main
	./main

