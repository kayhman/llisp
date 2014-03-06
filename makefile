
CPP=clang++
CFLAGS=-std=c++11 -stdlib=libc++ -lcxxrt -ldl -g
#CFLAGS=-std=c++11 -stdlib=libc++ -nodefaultlibs -lc++ -lcxxrt -lm -lc -lgcc_s -lgcc -ldl
#CFLAGS=-std=c++11 -stdlib=libc++ -lcxxrt -dl
#CFLAGS=-std=c++11 -stdlib=libc++ `llvm-config --libs core` `llvm-config --ldflags` 

all: elisp macro

elisp: cell.cpp core.cpp  string.cpp parse.cpp
	clear
	$(CPP) $(CFLAGS) -o $@ $?

macro: macro.cpp 
	clear
	$(CPP) $(CFLAGS) -o $@ $?


