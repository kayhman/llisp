#include <iostream>

extern "C" void hello() {
  std::cout << "hello " << std::endl;
}
