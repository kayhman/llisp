#include <ostream>
#include "cell.h"


extern "C" void registerSpecialHandlers(Cell::CellEnv& env)
{
  std::shared_ptr<Atom> eq = SymbolAtom::New(env, "eq");
  eq->closureType = [](Sexp* sexp, Cell::CellEnv& env) { return Cell::Type::Real; };
  eq->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> m1 = sexp->cells[1];
    std::shared_ptr<Cell> m2 = sexp->cells[2];
    
    std::shared_ptr<Cell> res = SymbolAtom::New();
    if (m1->eval(env)->val.compare(m2->eval(env)->val) == 0)
      res->real = 1.0;
    else
      res->real = 0.;
    return res;
  };

  std::shared_ptr<Atom> iff = SymbolAtom::New(env, "if");
  iff->closureType = [](Sexp* sexp, Cell::CellEnv& env) { 
    std::shared_ptr<Cell> test = sexp->cells[1];
    std::shared_ptr<Cell> True = sexp->cells[2];
    std::shared_ptr<Cell> False = sexp->cells[3];

    Cell::Type trueType = True->evalType(env);
    Cell::Type falseType = False->evalType(env);
    
    if(trueType == falseType)
      return trueType;
    else        
      return Cell::Type::Unknown; //TODO : add error message.
  };

  iff->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> test = sexp->cells[1];
    std::shared_ptr<Cell> True = sexp->cells[2];
    std::shared_ptr<Cell> False = sexp->cells[3];
    
    if(test->eval(env)->real)
      return True->eval(env);
    else        
      return False->eval(env);
  };

  std::shared_ptr<Atom> quote = SymbolAtom::New(env, "quote");
  quote->closure = [](Sexp* sexp, Cell::CellEnv& env) {
      return sexp->cells[1];
  };

  std::shared_ptr<Atom> funcode = SymbolAtom::New(env, "funcode");
  funcode->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    auto clIt = env.func.find(sexp->cells[1]->val);
    if(clIt != env.func.end())
      if(auto fun = std::dynamic_pointer_cast<SymbolAtom>(clIt->second))
	  return fun->code;
    return sexp->cells[0];
  };

  std::shared_ptr<Atom> backquote = SymbolAtom::New(env, "backquote");
  backquote->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::cout << "call backquote -> " << sexp->cells[1] << std::endl;
    return sexp->cells[1];
  };

  std::shared_ptr<Atom> progn = SymbolAtom::New(env, "progn");
  progn->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::for_each(sexp->cells.begin()+1, sexp->cells.end()-1, [&](std::shared_ptr<Cell> cell){cell->eval(env);});
    return sexp->cells.back()->eval(env);
  };

  std::shared_ptr<Atom> setq = SymbolAtom::New(env, "setq");
  setq->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    auto it = env.find(sexp->cells[1]->val);
    auto res = sexp->cells[2]->eval(env);

    if(it != env.end())
      {
	it->second.reset();
	it->second = res;
      }
    else
      {
	env[sexp->cells[1]->val] = res;
      }
    return res;
  };

  std::shared_ptr<Atom> let = SymbolAtom::New(env, "let");
  let->closure = [](Sexp* sexp, Cell::CellEnv& env) {
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
