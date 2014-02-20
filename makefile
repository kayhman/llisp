
CPP=g++
CFLAGS=-std=c++11

elisp: parse.cpp
	clear
	$(CPP) $(CFLAGS) -o $@ $?


