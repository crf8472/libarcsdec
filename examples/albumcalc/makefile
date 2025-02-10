# Makefile for the example 'albumcalc', part of libarcsdec

CXX     := g++
## Replace 'g++' with 'clang++' if you like to use clang
CFLAGS  := -std=c++17 -Wall -Wextra -pedantic
LDFLAGS := "-Wl,--disable-new-dtags,-rpath,../../build/,-rpath,../../build/libs/libarcstk"
## We force to use libarcsdec.so from build tree. Check this with
## $ ldd albumcalc | grep arcs
## or
## $ readelf -d albumcalc | head -20
## and look for the value of RPATH.

PROJECT := albumcalc

## Uncomment exactly one of the following pairs to definitions of LIBARCSTK:

## Uncomment this version for use with libarcstk as a submodule
LIBARCSTK_SO      := ../../build/libs/libarcstk/libarcstk.so
LIBARCSTK_HEADERS := ../../build/libs/libarcstk/include

## Uncomment this version for use with libarcstk installed on system
#LIBARCSTK_SO      := -larcstk
#LIBARCSTK_HEADERS :=

%.o : %.cpp
	$(CXX) $(CFLAGS) -c -I../../build/include -I$(LIBARCSTK_HEADERS) $<

$(PROJECT) : $(PROJECT).o
	$(CXX) $< $(LDFLAGS) ../../build/libarcsdec.so $(LIBARCSTK_SO) -o $@

clean:
	rm -f *.o $(PROJECT)

