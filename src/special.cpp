#include <ostream>
#include "cell.h"

double equal(std::shared_ptr<Cell> m1, std::shared_ptr<Cell> m2, Cell::CellEnv& env)
{
  if(m1->evalType(env) != m1->evalType(env)) {
    return 0.;
  }
  
  if(m1->evalType(env) == Cell::Type::String) {
    if (m1->eval(env)->val.compare(m2->eval(env)->val) == 0)
      return 1.;
    else
      return 0.;
  }
  
  if(m1->evalType(env) == Cell::Type::Real) {
    if (m1->eval(env)->real == m2->eval(env)->real)
      return 1.;
    else
      return 0.;
  }
  
  if(m1->evalType(env) == Cell::Type::List) {
    std::shared_ptr<Sexp> sm1 = std::dynamic_pointer_cast<Sexp>(m1);
    std::shared_ptr<Sexp> sm2 = std::dynamic_pointer_cast<Sexp>(m2);
    std::vector<std::shared_ptr<Cell> >::iterator m1It;
    std::vector<std::shared_ptr<Cell> >::iterator m2It;
    for(m1It = sm1->cells.begin(), 
          m2It = sm2->cells.begin() ;
        m1It != sm1->cells.end() &&  m2It != sm2->cells.end();
        m1It++, m2It++)
      if(!equal(*m1It, *m2It, env))
        return 0;
    return 1.0;
  }
  return 0.0;
}


extern "C" void registerSpecialHandlers(Cell::CellEnv& env)
{
  std::shared_ptr<Atom> ext = SymbolAtom::New(env, "exit");
  ext->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    exit(0);
    return RealAtom::New();
  };

  std::shared_ptr<Atom> eq = SymbolAtom::New(env, "eq");
  eq->closureType = [](Sexp* sexp, Cell::CellEnv& env) { return Cell::Type::Real; };
  eq->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> m1 = sexp->cells[1]->eval(env);
    std::shared_ptr<Cell> m2 = sexp->cells[2]->eval(env);
    
    std::shared_ptr<Cell> res;
    if(equal(m1, m2, env))
      res = Cell::t;
    else
      res = Cell::nil;

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


  std::shared_ptr<Atom> cond = SymbolAtom::New(env, "cond");
  cond->closureType = [](Sexp* sexp, Cell::CellEnv& env) { 
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

  cond->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    for(auto condIt = sexp->cells.begin() + 1 ; condIt != sexp->cells.end() ; condIt++)
      {
        std::shared_ptr<Sexp> cond = std::dynamic_pointer_cast<Sexp>(*condIt);
        std::shared_ptr<Cell> test = cond->cells[0];
        if(test->eval(env)->real) {
          std::shared_ptr<Cell> res;
          for(auto codeIt = cond->cells.begin() + 1 ; codeIt != cond->cells.end() ; codeIt++)
            res = (*codeIt)->eval(env);
          return res;
        }
      }
    return Cell::nil;
  };

  std::shared_ptr<Atom> when = SymbolAtom::New(env, "when");
  when->closureType = [](Sexp* sexp, Cell::CellEnv& env) { 
    std::shared_ptr<Cell> test = sexp->cells[1];
    std::shared_ptr<Cell> True = sexp->cells[2];

    Cell::Type trueType = True->evalType(env);
    return trueType;
  };

  when->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    if(sexp->cells.size() == 3) {
      std::shared_ptr<Cell> test = sexp->cells[1];
      std::shared_ptr<Cell> True = sexp->cells[2];
      
      if(test->eval(env)->real)
	return True->eval(env);
      else 
	return Cell::nil;
    }
    else {
      std::shared_ptr<Cell> test = sexp->cells[1];
      if(test->eval(env)->real)
	return Cell::t;
      else 
	return Cell::nil;
    }

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
    std::function<void(std::shared_ptr<Cell>& c)> recursiveReplace = [&](std::shared_ptr<Cell>& cellPtr)
    {
      Sexp* sxp = dynamic_cast<Sexp*>(cellPtr.get());
      
      if(sxp) {
        if(sxp->cells.size())
          {
            if(sxp->cells[0]->val == "comma")
              {
                std::shared_ptr<Cell> newCell = sxp->eval(env);
                cellPtr = newCell;
              }
            else {
              for(auto cIt = sxp->cells.begin() ; cIt != sxp->cells.end() ; cIt++) {
                if(dynamic_cast<Sexp*>(cIt->get()))
                  recursiveReplace(*cIt);
              } 
            }
          }
      }
    };

    if(dynamic_cast<Sexp*> (sexp->cells[1].get()))
      recursiveReplace(sexp->cells[1]);

    return sexp->cells[1];
  };

  std::shared_ptr<Atom> comma = SymbolAtom::New(env, "comma");
  comma->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    Sexp* sxp = dynamic_cast<Sexp*>(sexp->cells[1].get());

    if(sxp) {
      return sexp->cells[1]; //->eval(env);
    } else {
      return env[sexp->cells[1]->val];
    }
      
  };

  std::shared_ptr<Atom> progn = SymbolAtom::New(env, "progn");
  progn->closureType = [](Sexp* sexp, Cell::CellEnv& env) { 
    return sexp->cells.back()->evalType(env);
  };


  progn->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::for_each(sexp->cells.begin()+1, sexp->cells.end()-1, [&](std::shared_ptr<Cell> cell){cell->eval(env);});
    return sexp->cells.back()->eval(env);
  };

  std::shared_ptr<Atom> andd = SymbolAtom::New(env, "and");
  andd->closureType = [](Sexp* sexp, Cell::CellEnv& env) { 
    return sexp->cells.back()->evalType(env);
  };
  
  andd->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> res;
    for(auto cIt = sexp->cells.begin()+1 ;cIt != sexp->cells.end() ; cIt++) {
      res = (*cIt)->eval(env);
      if(res->evalType(env) == Cell::Type::Real)
        if(res->real == 0)
          break;
    }
    return res;
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
	env.top[sexp->cells[1]->val] = res;
      }

    return res;
  };

  std::shared_ptr<Atom> let = SymbolAtom::New(env, "let");
  let->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Sexp> vars = std::dynamic_pointer_cast<Sexp>(sexp->cells[1]);
    
    std::map<std::string, std::shared_ptr<Cell> > newEnv;
    for(int vId = 0 ; vId < vars->cells.size() ; vId++) {
      std::shared_ptr<Sexp> binding = std::dynamic_pointer_cast<Sexp>(vars->cells[vId]);
      std::shared_ptr<Atom> label = std::dynamic_pointer_cast<Atom>(binding->cells[0]);
      std::shared_ptr<Cell> value = std::dynamic_pointer_cast<Cell>(binding->cells[1]);
      
      newEnv[label->val].reset();
      
      const std::shared_ptr<Cell> res = value->eval(env);
      newEnv[label->val] = res;
    }
    
    env.addEnvMap(&newEnv);
    std::shared_ptr<Cell> res;
    for(auto bodyIt = sexp->cells.begin() + 2 ; bodyIt != sexp->cells.end() ; bodyIt++) {
      res = (*bodyIt)->eval(env);
    }
    env.removeEnv();
    if (res)
      return res;
    else
      return Cell::nil;
  };

  std::shared_ptr<Atom> letStart = SymbolAtom::New(env, "let*");
  letStart->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Sexp> vars = std::dynamic_pointer_cast<Sexp>(sexp->cells[1]);
    
    std::map<std::string, std::shared_ptr<Cell> > newEnv;
    env.addEnvMap(&newEnv);
    for(int vId = 0 ; vId < vars->cells.size() ; vId++) {
      std::shared_ptr<Sexp> binding = std::dynamic_pointer_cast<Sexp>(vars->cells[vId]);
      std::shared_ptr<Atom> label = std::dynamic_pointer_cast<Atom>(binding->cells[0]);
      std::shared_ptr<Cell> value = std::dynamic_pointer_cast<Cell>(binding->cells[1]);
      
      newEnv[label->val].reset();
      
      const std::shared_ptr<Cell> res = value->eval(env);
      newEnv[label->val] = res;
    }
    
    std::shared_ptr<Cell> res;
    for(auto bodyIt = sexp->cells.begin() + 2 ; bodyIt != sexp->cells.end() ; bodyIt++) {
      res = (*bodyIt)->eval(env);
    }
    env.removeEnv();
    if (res)
      return res;
    else
      return Cell::nil;
  };

  std::shared_ptr<Atom> printenv = SymbolAtom::New(env, "printenv");
  printenv->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> res = StringAtom::New();
    res->val = "printenv";
    std::cout << env << std::endl;
    return res;
  };
}
