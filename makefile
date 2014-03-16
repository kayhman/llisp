CPP=clang++
#CFLAGS=-std=c++11 -stdlib=libc++ -lcxxrt -ldl -g
CFLAGS=-std=c++11 -stdlib=libc++ -lcxxrt -ldl -O3

all: libenvironment.so libcell.so string.so core.so functional.so bench.so elisp
	sudo cp libenvironment.so libcell.so /usr/local/lib

clean:
	rm -rf *.so elisp

elisp: parse.cpp libenvironment.so libcell.so
	$(CPP) $(CFLAGS) -L. -lenvironment -lcell -o $@ $?

libenvironment.so: environment.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o $@ $?

libcell.so: cell.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -L. -lenvironment -o $@ $?

string.so: string.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o $@ $?

core.so: core.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o $@ $?

functional.so: functional.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o $@ $?

bench.so: bench.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o $@ $?
