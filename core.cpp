#include "core.h"
#include <ostream>
#include "cell.h"

void registerCoreHandlers()
{
  Cell::CellEnv env;

  env.evalHandlers["+"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> res(new Atom());
    double sum = 0;
    std::for_each(sexp->cells.begin()+1, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){sum += atof(cell->eval(env)->val.c_str());});
    std::ostringstream ss;
    ss << sum;
    res->val = ss.str();
    return res;
  };

  env.evalHandlers["-"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> res(new Atom());
    double sum = atof(sexp->cells[1]->eval(env)->val.c_str());
    std::for_each(sexp->cells.begin()+2, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){sum -= atof(cell->eval(env)->val.c_str());});

    std::ostringstream ss;
    ss << sum;
    res->val = ss.str();
    return res;
  };

  env.evalHandlers["*"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> res(new Atom());
    double prod = 1.0;
    std::for_each(sexp->cells.begin()+1, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){prod *= atof(cell->eval(env)->val.c_str());});
    std::ostringstream ss;
    ss << prod;
    res->val = ss.str();
    return res;
  };

  env.evalHandlers["/"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> res(new Atom());
    double quotient = atof(sexp->cells[1]->eval(env)->val.c_str());
    std::for_each(sexp->cells.begin()+2, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){quotient /= atof(cell->eval(env)->val.c_str());});
    std::ostringstream ss;
    ss << quotient;
    res->val = ss.str();
    return res;
  };

  env.evalHandlers["eq"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> m1 = sexp->cells[1];
    std::shared_ptr<Cell> m2 = sexp->cells[2];
    
    std::shared_ptr<Cell> res(new Atom());
    if (m1->eval(env)->val.compare(m2->eval(env)->val) == 0)
      {
	res->val = "True";
      }
    else
      {
	res->val = "";
      }
    return res;
  };

  env.evalHandlers["if"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> test = sexp->cells[1];
    std::shared_ptr<Cell> True = sexp->cells[2];
    std::shared_ptr<Cell> False = sexp->cells[3];
    
    if(test->eval(env)->val != "")
      return True->eval(env);
    else        
      return False->eval(env);
  };

  env.evalHandlers["quote"] = [](Sexp* sexp, Cell::CellEnv& env) {
      return sexp->cells[1];
  };

  env.evalHandlers["backquote"] = [](Sexp* sexp, Cell::CellEnv& env) {
    return sexp->cells[1];
  };

  env.evalHandlers["progn"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::for_each(sexp->cells.begin()+1, sexp->cells.end()-1, [&](std::shared_ptr<Cell> cell){cell->eval(env);});
    return sexp->cells.back()->eval(env);
  };
}
