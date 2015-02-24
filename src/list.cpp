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
  
  std::shared_ptr<Atom> car = SymbolAtom::New(env, "car");

  std::dynamic_pointer_cast<SymbolAtom>(car)->prototype = Prototype("ul");  
  car->closure = [](Sexp* sexp, Cell::CellEnv& env) {
		if(sexp->cells.size() == 2)
    {
			std::shared_ptr<Cell> content = sexp->cells[1]->eval(env);
			if(content->evalType(env) == Cell::List)
				return static_cast<Sexp*>(content.get())->cells[0];
			else
				return Cell::nil;
   }
   else
    return Cell::nil;
  };
  
	std::shared_ptr<Atom> cdr = SymbolAtom::New(env, "cdr");

  std::dynamic_pointer_cast<SymbolAtom>(cdr)->prototype = Prototype("ll");  
  cdr->closure = [](Sexp* sexp, Cell::CellEnv& env) {
		if(sexp->cells.size() == 2)
    {
			std::shared_ptr<Cell> content = sexp->cells[1]->eval(env);
			if(content->evalType(env) == Cell::List)
      {
       Sexp * sres = new Sexp();
 			 sres->cells = std::vector<std::shared_ptr<Cell> >(static_cast<Sexp*>(content.get())->cells.begin()+1, static_cast<Sexp*>(content.get())->cells.end());
       std::shared_ptr<Cell> res(sres);

				return res;
			}
			else
				return Cell::nil;
   }
   else
    return Cell::nil;
  };

}
