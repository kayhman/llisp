clang++ -c -emit-llvm test.cpp  -o test.ll
llc-3.4 -march=cpp -o test.ll.cpp test.ll
