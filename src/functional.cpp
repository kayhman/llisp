#include "cell.h"
#include <sstream>

extern "C" void registerFunctionalHandlers(Cell::CellEnv& env)
{
  std::shared_ptr<Atom> defun = SymbolAtom::New(env, "defun"); //handle recursion
  defun->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Atom> fname = SymbolAtom::New(env, sexp->cells[1]->val);
    std::shared_ptr<Sexp> args = std::dynamic_pointer_cast<Sexp>(sexp->cells[2]);
    std::shared_ptr<Sexp> body = std::dynamic_pointer_cast<Sexp>(sexp->cells[3]);
    std::dynamic_pointer_cast<SymbolAtom>(fname)->code = body;
    std::dynamic_pointer_cast<SymbolAtom>(fname)->args = args;
    if(args && body)
      {

	fname->closure = [env, args, body](Sexp* self, Cell::CellEnv& callingEnv) mutable {
	  if(&env != &callingEnv)
	    for(auto eIt = callingEnv.envs.begin() ; eIt != callingEnv.envs.end() ; ++eIt)
	      {
		env.addEnvMap(*eIt);
	      }
	  
	  std::map<std::string, std::shared_ptr<Cell> > newEnv;
	  for(int c = 0 ; c < args->cells.size() ; c++)
	    {
	      std::shared_ptr<Cell> val = self->cells[c+1]->eval(env);
	      std::shared_ptr<SymbolAtom> symb = std::dynamic_pointer_cast<SymbolAtom>(val);
	      if(symb && env.find(symb->val) != env.end())
		newEnv[args->cells[c]->val] = env[symb->val];
	      else
		newEnv[args->cells[c]->val] = val; // Eval args before adding them to env (avoid infinite loop when defining recursive function)
	    }
          env.addEnvMap(&newEnv);
	  std::shared_ptr<Cell> res = body->eval(env);
	  //The following lines are useless :
	  env.removeEnv();
	  if(&env != &callingEnv)
	    for(auto eIt = callingEnv.envs.begin() ; eIt != callingEnv.envs.end() ; eIt++)
	      env.removeEnv();
	  return res;
	};

      }

    return fname;
  };

  std::shared_ptr<Atom> lambda = SymbolAtom::New(env, "lambda");
  lambda->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Atom> lambda = std::dynamic_pointer_cast<Atom>(sexp->cells[0]);
    std::shared_ptr<Sexp> args =  std::dynamic_pointer_cast<Sexp>(sexp->cells[1]);
    std::shared_ptr<Sexp> body =  std::dynamic_pointer_cast<Sexp>(sexp->cells[2]);
    std::shared_ptr<Cell> res = SymbolAtom::New();

    if(args && body) {
      res->closure = [env, args, body](Sexp* self, Cell::CellEnv& callingEnv) mutable {
        if(&env != &callingEnv)
          for(auto eIt = callingEnv.envs.begin() ; eIt != callingEnv.envs.end() ; ++eIt)
	      {
		env.addEnvMap(*eIt);
	      }
	  
	  std::map<std::string, std::shared_ptr<Cell> > newEnv;
	  for(int c = 0 ; c < args->cells.size() ; c++)
	    {
	      std::shared_ptr<Cell> val = self->cells[c+1]->eval(env);
	      std::shared_ptr<SymbolAtom> symb = std::dynamic_pointer_cast<SymbolAtom>(val);
	      if(symb && env.find(symb->val) != env.end())
		newEnv[args->cells[c]->val] = env[symb->val];
	      else
		newEnv[args->cells[c]->val] = val; // Eval args before adding them to env (avoid infinite loop when defining recursive function)
	    }
	  env.addEnvMap(&newEnv);
	  std::shared_ptr<Cell> res = body->eval(env);
	  
	  //The following lines are useless :
	  env.removeEnv();
	  if(&env != &callingEnv)
	    for(auto eIt = callingEnv.envs.begin() ; eIt != callingEnv.envs.end() ; eIt++)
	      env.removeEnv();
	  return res;
	};
    }
        
    
    res->val = "lambda closure";
    res->real = 1;
    
    return res;
  };

  std::shared_ptr<Atom> funcall = SymbolAtom::New(env, "funcall");
  funcall->closure = [](Sexp* sexp, Cell::CellEnv& env)  {
    std::shared_ptr<Cell> lambda = std::dynamic_pointer_cast<Cell>(sexp->cells[1])->eval(env);
    std::shared_ptr<Sexp> args = Sexp::New();
    for(auto aIt = sexp->cells.begin()+1 ; aIt != sexp->cells.end() ; aIt++)
      args->cells.push_back(*aIt);

    return lambda->closure(args.get(), env);
  };

  std::shared_ptr<Atom> defmacro = SymbolAtom::New(env, "defmacro");
  defmacro->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Atom> fname = SymbolAtom::New(env, sexp->cells[1]->val);
    std::shared_ptr<Sexp> args = std::dynamic_pointer_cast<Sexp>(sexp->cells[2]);
    std::shared_ptr<Cell> body = sexp->cells[3];
    
    if(args && body)
      {
	fname->closure = [env, args, body, fname](Sexp* self, Cell::CellEnv& callingEnv) mutable {
          if(&env != &callingEnv)
	    for(auto eIt = callingEnv.envs.begin() ; eIt != callingEnv.envs.end() ; ++eIt)
	      {
		env.addEnvMap(*eIt);
	      }

	  //assert(cls.size() == args->cells.size());
	  std::vector<std::shared_ptr<Cell> > cls(self->cells.begin()+1, self->cells.end());	  
	  std::stringstream ss;
	  
	  std::map<std::string, std::shared_ptr<Cell> > newEnv;
	  for(int c = 0 ; c < args->cells.size() ; c++)
	    {
	      std::shared_ptr<Cell> val = self->cells[c+1];//don't eval as we are in a macro !
	      std::shared_ptr<SymbolAtom> symb = std::dynamic_pointer_cast<SymbolAtom>(val);
	      if(symb && env.find(symb->val) != env.end())
		newEnv[args->cells[c]->val] = env[symb->val];
	      else
		newEnv[args->cells[c]->val] = val; // Eval args before adding them to env (avoid infinite loop when defining recursive function)
	    }
	  env.addEnvMap(&newEnv);
          
          // We perform the macro expansion
          std::function<void(std::shared_ptr<Cell>& c)> recursiveReplace = [&](std::shared_ptr<Cell>& cellPtr)
          {
            Sexp* sxp = dynamic_cast<Sexp*>(cellPtr.get());
            
            if(sxp) {
              if(sxp->cells.size())
                {
                  if(sxp->cells[0]->val == "backquote")
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

          
          
          std::shared_ptr<Cell> copy = body; //->duplicate();
          if(dynamic_cast<Sexp*>(copy.get())) {
            recursiveReplace(copy);
          }

          std::shared_ptr<Cell> res = copy->eval(env);          
	  //The following lines are useless :
	  if(&env != &callingEnv)
	    for(auto eIt = callingEnv.envs.begin() ; eIt != callingEnv.envs.end() ; eIt++)
              env.removeEnv();
          
	  return res;
	};
      }
    env.func[fname->val] = fname;
    return fname;
  };
}
