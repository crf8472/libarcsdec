# Makefile for the example 'albumcalc', part of libarcsdec

CXX     := g++
## Replace 'g++' with 'clang++' if you like to use clang
CFLAGS  := -std=c++14 -Wall -Wextra -pedantic -larcstk

PROJECT := albumcalc

%.o : %.cpp
	$(CXX) -c $(CFLAGS) -O3 $<

$(PROJECT) : $(PROJECT).o
	$(CXX) $(CFLAGS) $< -o $@ ../../build/libarcsdec.so -lcue -lsndfile

clean:
	rm -f *.o $(PROJECT)
