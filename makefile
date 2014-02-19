
CPP=g++
CFLAGS=-std=c++11

elisp: parse.cpp
	$(CPP) $(CFLAGS) -o $@ $?


