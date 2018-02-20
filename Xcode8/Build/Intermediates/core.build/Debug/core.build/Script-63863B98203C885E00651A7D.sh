#!/bin/sh
bison -d -v -t -o ../../core/pbrtparse.cpp ../../core/pbrtparse.y
/bin/mv ../../core/pbrtparse.hpp ../../core/pbrtparse.h
flex -o../../core/pbrtlex.cpp ../../core/pbrtlex.l
