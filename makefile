CPP=g++
CFLAGS=-std=c++11 -ldl -O3
LLVM_CFLAGS=`llvm-config --cxxflags`
LLVM_LIB=`llvm-config --libs`
LLVM_LINK=`llvm-config --ldflags`


all: libenvironment.so libcell.so string.so core.so functional.so bench.so elisp
	sudo cp libenvironment.so libcell.so /usr/local/lib

clean:
	rm -rf *.so elisp

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

compiler: compiler.cpp
	$(CPP) $(LLVM_CFLAGS) $(LLVM_LINK) -o $@ $? $(LLVM_LIB) -pthread -ldl


elisp: parse.cpp compiler.cpp libenvironment.so libcell.so
	$(CPP) $(CFLAGS) $(LLVM_CFLAGS) $(LLVM_LINK) -o $@ compiler.cpp parse.cpp $(LLVM_LIB) -L. -lenvironment -lcell -pthread -ldl
