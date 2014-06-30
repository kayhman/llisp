#include "environment.h"
#include "cell.h"


extern "C" void registerStringHandlers(Cell::CellEnv& env)
{
  std::shared_ptr<Atom> concat = SymbolAtom::New(env, "concat");
  std::dynamic_pointer_cast<SymbolAtom>(concat)->prototype = Prototype("ss*");
  concat->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    //    std::shared_ptr<Cell> cl = sexp->cells[0];    
    std::shared_ptr<Cell> res = StringAtom::New();
    std::for_each(sexp->cells.begin()+1, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){res->val += cell->eval(env)->val;}); 
    return res;
  };

  std::shared_ptr<Atom> length = SymbolAtom::New(env, "length");
  std::dynamic_pointer_cast<SymbolAtom>(length)->prototype = Prototype("fs");
  length->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> string  = sexp->cells[1];    
    std::stringstream ss;
    std::shared_ptr<Cell> res = RealAtom::New();
    res->real = string->eval(env)->val.size();
    ss << res->real;
    res->val = ss.str();
    
    return res;
  };
}
