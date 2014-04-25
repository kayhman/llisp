g++ `llvm-config --cxxflags` -o hw.o -c hw.cpp
g++ `llvm-config --ldflags` -o hw hw.o `llvm-config --libs engine scalaropts` -pthread -ldl


clear
clang++ -std=c++11 `llvm-config --cxxflags` -o run-bc.o -c run-bc.cpp
clang++ -std=c++11 `llvm-config --ldflags` -o run-bc run-bc.o `llvm-config --libs engine scalaropts` -pthread -ldl


