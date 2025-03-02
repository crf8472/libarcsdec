# Makefile for the example 'albumcalc', part of libarcsdec

CXX     := g++
## Replace 'g++' with 'clang++' if you like to use clang
CFLAGS  := -std=c++17 -Wall -Wextra -pedantic
LDFLAGS := -Wl,--disable-new-dtags,-rpath,../../build/
## We force to use libarcsdec.so from build tree. Check this with
## $ ldd albumcalc | grep arcs
## or
## $ readelf -d albumcalc | head -20
## and look for the value of RPATH.

PROJECT := albumcalc

## Uncomment exactly ONE of the following blocks:

## Uncomment this block for use of libarcstk.so provided by submodule
LIBARCSTK_SO      := ../../build/libs/libarcstk/libarcstk.so
LIBARCSTK_HEADERS := ../../build/libs/libarcstk/include
LDFLAGS           := $(strip $(LDFLAGS)),-rpath,../../build/libs/libarcstk

## Uncomment this block for use of libarcstk.so provided by system installation
#LIBARCSTK_SO      := -larcstk   ## or use $ pkg-config --libs libarcstk
#LIBARCSTK_HEADERS :=

%.o : %.cpp
	$(CXX) $(CFLAGS) -c -I../../build/include -I$(LIBARCSTK_HEADERS) $<

$(PROJECT) : $(PROJECT).o
	$(CXX) $< "$(LDFLAGS)" ../../build/libarcsdec.so $(LIBARCSTK_SO) -o $@

clean:
	rm -f *.o $(PROJECT)

