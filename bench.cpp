#include "environment.h"
#include "cell.h"
#include <ctime>
#include <chrono>

extern "C" void registerBenchHandlers(Cell::CellEnv& env)
{
  std::shared_ptr<Atom> time = SymbolAtom::New(env, "time");
  std::dynamic_pointer_cast<SymbolAtom>(time)->prototype = Prototype("ff*");
  time->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> exp = sexp->cells[1];
    auto t_start = std::chrono::high_resolution_clock::now();
    std::shared_ptr<Cell> res = exp->eval(env);
    auto t_end = std::chrono::high_resolution_clock::now();
    std::cout << *exp << " takes " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count() << " ms" << std::endl;

    return res;
  };
}
