#include "cell.h"


extern "C" void registerFunctionalHandlers(Cell::CellEnv& env)
{
  env.evalHandlers["defun"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Atom> fname = std::dynamic_pointer_cast<Atom>(sexp->cells[1]); //weak
    std::shared_ptr<Sexp> args = std::dynamic_pointer_cast<Sexp>(sexp->cells[2]); //weak
    std::shared_ptr<Sexp> body = std::dynamic_pointer_cast<Sexp>(sexp->cells[3]); //weak
    
    if(args && body)
      {
	fname->closure = [env, args, body, fname](Cell* self, std::vector<std::shared_ptr<Cell> > cls) mutable {
	  //assert(cls.size() == args->cells.size());
	  std::map<std::string, std::shared_ptr<Cell> > newEnv;
	  for(int c = 0 ; c < cls.size() ; c++)
	    newEnv[args->cells[c]->val] = cls[c]->eval(env); // Eval args before adding them to env (avoir infinite loop when defining recursive function)
	  
	  //handle recursive call
	  newEnv[fname->val] = fname;
	  
	  env.addEnvMap(&newEnv);
	  std::shared_ptr<Cell> res = body->eval(env);
	  env.removeEnv();
	  return res;
	};
      }
    env.top[fname->val] = fname;
    return env.top[fname->val];
  };

  env.evalHandlers["lambda"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Atom> lambda = std::dynamic_pointer_cast<Atom>(sexp->cells[0]);
    std::shared_ptr<Sexp> args =  std::dynamic_pointer_cast<Sexp>(sexp->cells[1]);
    std::shared_ptr<Sexp> body =  std::dynamic_pointer_cast<Sexp>(sexp->cells[2]);
    
    if(args && body) {
      sexp->closure = [env,body, args](Cell* self, std::vector<std::shared_ptr<Cell> > cls) mutable {
	//assert(cls.size() == args->cells.size());
	std::map<std::string, std::shared_ptr<Cell> > newEnv;
	for(int c = 0 ; c < cls.size() ; c++)
	  newEnv[args->cells[c]->val] = cls[c];
	
	env.addEnvMap(&newEnv);
	std::shared_ptr<Cell> res = body->eval(env);
	env.removeEnv();
	return res;
      };
    }
    
    
    std::shared_ptr<Cell> res(new SymbolAtom());
    res->val = "closure";
    res->closure = sexp->closure;
    
    return res;
  };

  env.evalHandlers["funcall"] = [](Sexp* sexp, Cell::CellEnv& env)   {
    std::shared_ptr<Cell> lambda =  std::dynamic_pointer_cast<Cell>(sexp->cells[1])->eval(env);
    
    return  lambda->closure(lambda.get(), std::vector<std::shared_ptr<Cell> > (sexp->cells.begin()+2, sexp->cells.end()));
  };

  env.evalHandlers["defmacro"] = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Atom> fname = std::dynamic_pointer_cast<Atom>(sexp->cells[1]); //weak
    std::shared_ptr<Sexp> args = std::dynamic_pointer_cast<Sexp>(sexp->cells[2]); //weak
    std::shared_ptr<Cell> body = sexp->cells[3];
    
    if(args && body)
      {
	fname->closure = [env, args, body, fname](Cell* self, std::vector<std::shared_ptr<Cell> > cls) mutable {
	  //assert(cls.size() == args->cells.size());
	  
	  std::stringstream ss;
	  
	  std::function<std::shared_ptr<Cell>(std::shared_ptr<Cell>, std::regex re, std::string s)> recursiveReplace = [&](std::shared_ptr<Cell> cell, std::regex re, std::string s)
	  {
	    std::shared_ptr<Sexp> sexp = std::dynamic_pointer_cast<Sexp>(cell);
	    std::shared_ptr<Atom> atom = std::dynamic_pointer_cast<Atom>(cell);
	    
	    if(sexp)
	      {
		if(sexp->cells.size() > 0)
		  {
		    std::shared_ptr<Sexp> newS(new Sexp());
		    std::for_each(sexp->cells.begin(), sexp->cells.end(), [&](std::shared_ptr<Cell> cell){ newS->cells.push_back(recursiveReplace(cell, re, s));});
		    
		    if(sexp->cells[0]->val.compare("backquote") == 0)
		      return newS->eval(env);
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
	  std::cout << "macro expansion " << *body << std::endl;
	  
	  std::shared_ptr<Cell> res = body->eval(env);
	  
	  return res;
	};
      }
    env.top[fname->val] = fname;
    return env.top[fname->val];
  };
}
