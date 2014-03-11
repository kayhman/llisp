#include "environment.h"
#include "cell.h"
#include <ctime>


extern "C" void registerBenchHandlers(Cell::CellEnv& env)
{
  env.evalHandlers["time"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> exp = sexp->cells[1];
    clock_t start_time = clock();
    std::shared_ptr<Cell> res = exp->eval(env);
    clock_t finis_time = clock();
    std::cout << "cloeck " << start_time << " " << finis_time << std::endl;
    std::cout << *exp << " takes " << ((double) ((finis_time - start_time)) / CLOCKS_PER_SEC) <<  " seconds" << std::endl;

    return res;
  };
}
