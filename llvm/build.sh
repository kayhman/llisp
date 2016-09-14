clear
g++ `llvm-config-3.5 --cxxflags` -o hw.o -c hw.cpp
g++ `llvm-config-3.5 --ldflags` -o hw hw.o `llvm-config --libs` -pthread -ldl


#clear
#g++ -std=c++11 `llvm-config --cxxflags` -o run-bc.o -c run-bc.cpp
#g++ -std=c++11 `llvm-config --ldflags` -o run-bc run-bc.o `llvm-config --libs engine scalaropts` -pthread -ldl


