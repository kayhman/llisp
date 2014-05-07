#include "cell.h"


extern "C" void registerFunctionalHandlers(Cell::CellEnv& env)
{
  std::shared_ptr<Atom> defun = SymbolAtom::New(env, "defun"); //handle recursion
  defun->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Atom> fname = SymbolAtom::New(env, sexp->cells[1]->val);//std::dynamic_pointer_cast<Atom>(sexp->cells[1]); //weak
    std::shared_ptr<Sexp> args = std::dynamic_pointer_cast<Sexp>(sexp->cells[2]); //weak
    std::shared_ptr<Sexp> body = std::dynamic_pointer_cast<Sexp>(sexp->cells[3]); //weak
    std::dynamic_pointer_cast<SymbolAtom>(fname)->code = body;
    if(args && body)
      {

	fname->closure = [env, args, body, fname](Sexp* self, Cell::CellEnv& dummy) mutable {
	  //assert(cls.size() == args->cells.size());
	  std::map<std::string, std::shared_ptr<Cell> > newEnv;
	  for(int c = 0 ; c < args->cells.size() ; c++)
	    newEnv[args->cells[c]->val] = self->cells[c+1]->eval(env); // Eval args before adding them to env (avoid infinite loop when defining recursive function)
	  
	  env.addEnvMap(&newEnv);
	  std::shared_ptr<Cell> res = body->eval(env);
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
    
    if(args && body) {
      sexp->closure = [env,body, args](Sexp* self, Cell::CellEnv& dummy) mutable {
	std::map<std::string, std::shared_ptr<Cell> > newEnv;
	for(int c = 0 ; c < args->cells.size() ; c++)
	  newEnv[args->cells[c]->val] = self->cells[c+2];
	
	env.addEnvMap(&newEnv);
	std::shared_ptr<Cell> res = body->eval(env);
	env.removeEnv();
	return res;
      };
    }
        
    std::shared_ptr<Cell> res = SymbolAtom::New();
    res->val = "lambda closure";
    res->closure = sexp->closure;
    
    return res;
  };

  std::shared_ptr<Atom> funcall = SymbolAtom::New(env, "funcall");
  funcall->closure = [](Sexp* sexp, Cell::CellEnv& env)   {
    std::shared_ptr<Cell> lambda =  std::dynamic_pointer_cast<Cell>(sexp->cells[1])->eval(env);
    
    return  lambda->closure(sexp, env);
  };

  std::shared_ptr<Atom> defmacro = SymbolAtom::New(env, "defmacro");
  defmacro->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Atom> fname = SymbolAtom::New(env, sexp->cells[1]->val);//std::dynamic_pointer_cast<Atom>(sexp->cells[1]); //weak
    std::shared_ptr<Sexp> args = std::dynamic_pointer_cast<Sexp>(sexp->cells[2]); //weak
    std::shared_ptr<Cell> body = sexp->cells[3];
    
    if(args && body)
      {
	fname->closure = [env, args, body, fname](Sexp* self, Cell::CellEnv& dummy) mutable {
	  //assert(cls.size() == args->cells.size());
	  std::vector<std::shared_ptr<Cell> > cls(self->cells.begin()+1, self->cells.end());	  
	  std::stringstream ss;
	  
	  std::function<std::shared_ptr<Cell>(std::shared_ptr<Cell>, std::regex re, std::string s)> recursiveReplace = [&](std::shared_ptr<Cell> cell, std::regex re, std::string s)
	  {
	    std::shared_ptr<Sexp> sexp = std::dynamic_pointer_cast<Sexp>(cell);
	    std::shared_ptr<Atom> atom = std::dynamic_pointer_cast<Atom>(cell);
	    
	    if(sexp)
	      {
		if(sexp->cells.size() > 0)
		  {
		    std::shared_ptr<Sexp> newS = Sexp::New();
		    std::for_each(sexp->cells.begin(), sexp->cells.end(), [&](std::shared_ptr<Cell> cell){ newS->cells.push_back(recursiveReplace(cell, re, s));});
		    
		    if(sexp->cells[0]->val.compare("backquote") == 0)
		      return newS->cells[1]; //FIX : should call backquote symbol
		    else
		      return std::dynamic_pointer_cast<Cell>(newS);
		  }
	      } else if(atom)
	      {
		atom->val = regex_replace(atom->val, re, s);
		return atom->eval(env);
	      }
	  };

	  for(int c = 0 ; c < cls.size() ; c++)
	    {
	      ss.str("");
	      std::string var;
	      ss << *cls[c];
	      
	      
	      var = ss.str();
	      ss.str("");
	      ss << "," << *args->cells[c];
	      std::regex re(ss.str());
	      body = recursiveReplace(body, re, var);
	    }
	  Sexp* selfx = dynamic_cast<Sexp*>(self); //weak
	  std::shared_ptr<Sexp> bodyx = std::dynamic_pointer_cast<Sexp>(body); //weak
	  if(selfx && bodyx)
	    selfx->cells = bodyx->cells;

	  
	  std::shared_ptr<Cell> res = body->eval(env);
	  
	  return res;
	};
      }
    env.func[fname->val] = fname;
    return fname;
  };
}
