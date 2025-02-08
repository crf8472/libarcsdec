# Makefile for the example 'albumcalc', part of libarcsdec

CXX     := g++
## Replace 'g++' with 'clang++' if you like to use clang
CFLAGS  := -std=c++17 -Wall -Wextra -pedantic
LDFLAGS := "-Wl,--disable-new-dtags,-rpath,../../build/"
## We force to use libarcsdec.so from build tree. Check this with
## $ ldd albumcalc | grep arcs
## or
## $ readelf -d albumcalc | head -20
## and look for the value of RPATH.

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
