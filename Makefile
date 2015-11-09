# Makefile for the program

all:
	g++ -std=c++0x -fopenmp -O3 main.cpp

clean:
	rm a.out