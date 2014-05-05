CPP=g++
CFLAGS=-std=c++11 -ldl -O3
LLVM_CFLAGS=-I/usr/lib/llvm-3.5/include  -D_DEBUG -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -O3 -fomit-frame-pointer -fvisibility-inlines-hidden -fno-exceptions -fPIC -Woverloaded-virtual -ffunction-sections -fdata-sections -Wcast-qual
LLVM_CFLAGS=`llvm-config-3.5 --cflags`
LLVM_LIB=`llvm-config-3.5 --libs`
LLVM_LINK=`llvm-config-3.5 --ldflags`


all: libenvironment.so libcell.so string.so core.so functional.so bench.so compiler.so elisp
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

compiler.so: compiler.cpp
	$(CPP) $(CFLAGS) $(LLVM_CFLAGS) $(LLVM_LINK) --shared -fPIC -o $@ $? $(LLVM_LIB) -pthread -ldl

#compiler: compiler.cpp
#	$(CPP) $(LLVM_CFLAGS) $(LLVM_LINK) -o $@ $? $(LLVM_LIB) -pthread -ldl


elisp: parse.cpp compiler.cpp libenvironment.so libcell.so
	$(CPP) $(CFLAGS) $(LLVM_CFLAGS) $(LLVM_LINK) -o $@ parse.cpp $(LLVM_LIB) -L. -lenvironment -lcell -pthread -ldl
