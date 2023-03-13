# Makefile for the example 'albumcalc', part of libarcsdec

CXX     := g++
## Replace 'g++' with 'clang++' if you like to use clang
CFLAGS  := -std=c++14 -Wall -Wextra -pedantic

PROJECT := albumcalc

## Uncomment exactly one of the following to definitions of LIBARCSTK:

## Uncomment this version for use with libarcstk as a submodule
LIBARCSTK := ../../build/libs/libarcstk/libarcstk.so

## Uncomment this version for use with libarcstk installed on system
#LIBARCSTK := -larcstk

%.o : %.cpp
	$(CXX) -c $(CFLAGS) -O3 $<

$(PROJECT) : $(PROJECT).o
	$(CXX) $(CFLAGS) $< -o $@ ../../build/libarcsdec.so $(LIBARCSTK)

clean:
	rm -f *.o $(PROJECT)
