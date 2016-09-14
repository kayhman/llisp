CPP=g++
CFLAGS=-g -std=c++11 -ldl -O3
LLVM_CFLAGS=-I/usr/lib/llvm-3.5/include  -D_DEBUG -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -O3 -fomit-frame-pointer -fvisibility-inlines-hidden -fno-exceptions -fPIC -Woverloaded-virtual -ffunction-sections -fdata-sections -Wcast-qual
LLVM_CFLAGS=`llvm-config-3.5 --cflags`
LLVM_LIB=`llvm-config-3.5 --libs`
LLVM_LINK=`llvm-config-3.5 --ldflags`
TESTS=$(wildcard tests/*.cl)

all: libenvironment.so libcell.so string.so list.so special.so core.so native.so functional.so bench.so compiler.so debug.so llisp

test: $(TESTS)

.PHONY: $(TESTS)
$(TESTS):
	@./llisp $@ -q


clean:
	rm -rf *.so llisp

libenvironment.so: src/environment.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o $@ $?

libcell.so: src/cell.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -L. -lenvironment -o $@ $?

string.so: src/string.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o $@ $?

list.so: src/list.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o $@ $?

core.so: src/core.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o $@ $?

native.so: src/native.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o $@ $?

special.so: src/special.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o $@ $?

functional.so: src/functional.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o $@ $?

bench.so: src/bench.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o $@ $?

debug.so: src/debug.cpp
	$(CPP) $(CFLAGS) --shared -fPIC -o $@ $?

compiler.so: src/compiler.cpp
	$(CPP) $(CFLAGS) $(LLVM_CFLAGS) $(LLVM_LINK) --shared -fPIC -o $@ $? $(LLVM_LIB) -pthread -ldl -lffi -L. -lenvironment

llisp: src/parse.cpp src/llisp.cpp
	$(CPP) $(CFLAGS) $(LLVM_CFLAGS) $(LLVM_LINK) -o $@ $? $(LLVM_LIB) -L. -lenvironment -lcell -pthread -ldl -lreadline

hw: llvm/hw.cpp
	$(CPP) $(CFLAGS) $(LLVM_CFLAGS) $(LLVM_LINK) -o $@ $? $(LLVM_LIB) -L. -lenvironment -lcell -pthread -ldl -lreadline -lffi -ltinfo

