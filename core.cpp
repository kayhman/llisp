#include <ostream>
#include "cell.h"

extern "C" void registerCoreHandlers(Cell::CellEnv env)
{
  env.evalHandlers["+"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> res(new RealAtom);
    res->real = 0;
    std::for_each(sexp->cells.begin()+1, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){res->real += cell->eval(env)->real;});

    return res;
  };

  env.evalHandlers["-"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> res(new RealAtom);
    res->real = sexp->cells[1]->eval(env)->real;
    std::for_each(sexp->cells.begin()+2, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){res->real -= cell->eval(env)->real;});

    return res;
  };

  env.evalHandlers["*"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> res(new RealAtom);
    res->real = 1.0;
    std::for_each(sexp->cells.begin()+1, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){res->real *= cell->eval(env)->real;});

    return res;
  };

  env.evalHandlers["/"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> res(new RealAtom);
    res->real = sexp->cells[1]->eval(env)->real;
    std::for_each(sexp->cells.begin()+2, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){res->real /= cell->eval(env)->real;});
    return res;
  };

   env.evalHandlers["<"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> m1 = sexp->cells[1];
    std::shared_ptr<Cell> m2 = sexp->cells[2];
   
    double a1 =m1->eval(env)->real;
    double a2 =m2->eval(env)->real;
    
    std::shared_ptr<Cell> res(new SymbolAtom);
    if (a1 < a2)
      res->val = "True";
    else
      res->val = "";
    return res;
  };

   env.evalHandlers[">"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> m1 = sexp->cells[1];
    std::shared_ptr<Cell> m2 = sexp->cells[2];
   
    double a1 = m1->eval(env)->real;
    double a2 = m2->eval(env)->real;
 
    std::shared_ptr<Cell> res(new SymbolAtom);
    if (a1 > a2)
      res->val = "True";
    else
      res->val = "";
    return res;
  };


  env.evalHandlers["eq"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> m1 = sexp->cells[1];
    std::shared_ptr<Cell> m2 = sexp->cells[2];
    
    std::shared_ptr<Cell> res(new SymbolAtom);
    if (m1->eval(env)->val.compare(m2->eval(env)->val) == 0)
      res->val = "True";
    else
      res->val = "";
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

  env.evalHandlers["setq"] = [](Sexp* sexp, Cell::CellEnv& env) {
    auto it = env.find(sexp->cells[1]->val);
    if(it != env.end())
      {
	it->second.reset();//reset(new Atom);
	it->second = sexp->cells[2]->eval(env);
      }
    else
      env[sexp->cells[1]->val] = sexp->cells[2]->eval(env);
    return env[sexp->cells[1]->val];
  };

  env.evalHandlers["let"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Sexp> vars = std::dynamic_pointer_cast<Sexp>(sexp->cells[1]);
    std::shared_ptr<Sexp> body = std::dynamic_pointer_cast<Sexp>(sexp->cells[2]);
    
    std::map<std::string, std::shared_ptr<Cell> >* newEnv = new std::map<std::string, std::shared_ptr<Cell> >();
    for(int vId = 0 ; vId < vars->cells.size() ; vId++) {
      std::shared_ptr<Sexp> binding = std::dynamic_pointer_cast<Sexp>(vars->cells[vId]);
      std::shared_ptr<Atom> label = std::dynamic_pointer_cast<Atom>(binding->cells[0]);
      std::shared_ptr<Atom> value = std::dynamic_pointer_cast<Atom>(binding->cells[1]);
      
      (*newEnv)[label->val].reset();//(new Atom);
      
      const std::shared_ptr<Cell> res =  value->eval(env);
      (*newEnv)[label->val] = res;
    }
    
    env.addEnvMap(newEnv);
    std::shared_ptr<Cell> res = body->eval(env);
    env.removeEnv();
    return res;
  };
  
}
