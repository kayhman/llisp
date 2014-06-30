#include "environment.h"
#include "cell.h"

extern "C" void registerListHandlers(Cell::CellEnv& env)
{
  std::shared_ptr<Atom> append = SymbolAtom::New(env, "append");
  std::dynamic_pointer_cast<SymbolAtom>(append)->prototype = Prototype("ll*");  
  append->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    Sexp* sres = new Sexp();
    std::shared_ptr<Cell> res(sres);
    std::for_each(sexp->cells.begin()+1, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){
	
	std::shared_ptr<Sexp> sx = std::dynamic_pointer_cast<Sexp>(cell->eval(env));
	if(sx)
	  std::for_each(sx->cells.begin(), sx->cells.end(), [&](std::shared_ptr<Cell> cl) {
	      sres->cells.push_back(cl);
	    });
      }); 
    return res;
  };
  
  std::shared_ptr<Atom> list = SymbolAtom::New(env, "list");

  std::dynamic_pointer_cast<SymbolAtom>(list)->prototype = Prototype("ll*");  
  list->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    Sexp * sres = new Sexp();
    std::shared_ptr<Cell> res(sres);
    std::for_each(sexp->cells.begin()+1, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){sres->cells.push_back(cell->eval(env));});
    return res;
  };
}
