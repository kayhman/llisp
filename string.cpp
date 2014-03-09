#include "environment.h"
#include "cell.h"
//#include "string.h"

extern "C" void registerStringHandlers(Cell::CellEnv& env)
{
  std::cout << "add concat handler" << std::endl;
  env.evalHandlers["concat"] = [](Sexp* sexp, Cell::CellEnv& env) {
    //    std::shared_ptr<Cell> cl = sexp->cells[0];    
    std::shared_ptr<Cell> res(new Atom);
    std::for_each(sexp->cells.begin()+1, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){res->val += cell->eval(env)->val;}); 
    return res;
  };

env.evalHandlers["length"] = [](Sexp* sexp, Cell::CellEnv& env) {
  std::shared_ptr<Cell> string  = sexp->cells[1];    
  std::stringstream ss;
  std::shared_ptr<Cell> res(new Atom);
  ss << string->eval(env)->val.size();
  res->val = ss.str();
  return res;
  };

  std::cout << "add concat handler (done)" << std::endl;
}
