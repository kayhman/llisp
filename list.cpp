#include "list.h"

void registerListHandlers()
{
  Cell::CellEnv env;
  
  env.evalHandlers["append"] = [](Sexp* sexp, Cell::CellEnv& env) {
    Sexp* sexp = new Sexp();
    std::shared_ptr<Cell> res(sexp);
    std::for_each(cells.begin()+1, cells.end(), [&](std::shared_ptr<Cell> cell){
	
	std::shared_ptr<Sexp> sx = std::dynamic_pointer_cast<Sexp>(cell->eval(env));
	if(sx)
	  std::for_each(sx->cells.begin(), sx->cells.end(), [&](std::shared_ptr<Cell> cl) {
	      sexp->cells.push_back(cl);
	    });
      }); 
    
    return res;
  };
  
  env.evalHandlers["list"] = [](Sexp* sexp, Cell::CellEnv& env) {
    Sexp * sexp = new Sexp();
    std::shared_ptr<Cell> res(sexp);
    std::for_each(cells.begin()+1, cells.end(), [&](std::shared_ptr<Cell> cell){sexp->cells.push_back(cell->eval(env));});
    return res;
  };
}
