# Makefile for the program

all:
	g++ -std=c++0x -pthread -O3 main.cpp

clean:
	rm a.out
