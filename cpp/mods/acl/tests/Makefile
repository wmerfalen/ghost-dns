GCC = g++
FLAGS = -std=c++14 -Wall -Werror
OUTDIR = build

fileparser: fileparser.o
	$(GCC) $(FLAGS) fileparser.o testfileparser.cpp -o $(OUTDIR)/fileparser

fileparser.o:
	$(GCC) $(FLAGS) -c ../config-parser.cpp -o fileparser.o

genstr: genstr.cpp
	$(GCC) $(FLAGS) genstr.cpp -o genstr

