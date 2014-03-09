CPP=clang++
CFLAGS=-std=c++11 -stdlib=libc++ -lcxxrt -ldl -g

all: libenvironment libcell libstring elisp
	sudo cp libenvironment.so libcell.so /usr/local/lib

clean:
	rm -rf *.so elisp

elisp: functional.cpp core.cpp parse.cpp
	$(CPP) $(CFLAGS) -L. -lenvironment -lcell -o $@ $?

libenvironment: environment.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o $@.so $?

libcell: cell.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -L. -lenvironment -o $@.so $?

libstring: string.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o $@.so $?


macro: macro.cpp 
	$(CPP) $(CFLAGS) -o $@ $?

loading: hello.cpp main.cpp
	$(CPP) $(CFLAGS) -o $@ main.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o hello.so hello.cpp
