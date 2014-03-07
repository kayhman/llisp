#include "environment.h"
#include "cell.h"

void registerStringHandlers()
{
  Cell::CellEnv env;
  
  env.evalHandlers["concat"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> cl = sexp->cells[0];    
    std::shared_ptr<Cell> res(new Atom);
    std::for_each(sexp->cells.begin()+1, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){res->val += cell->eval(env)->val;}); 
    return res;
  };
}
