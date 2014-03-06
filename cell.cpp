#include "cell.h"

template<typename K, typename V> std::map<K, V> Env<K,V>::top;
template<typename K, typename V> std::map<K, std::function<V(Sexp* sexp, Env<K,V>& env)> > Env<K,V>::evalHandlers;


bool isoperator(char c) {
     std::string const valid_chars = "+*-/!=<>\"'`,";
     return valid_chars.find(c) != std::string::npos;
}

std::ostream& operator<< (std::ostream& stream, const Cell& cell)
{
  const Atom * atom = dynamic_cast<const Atom*>(&cell);
  const Sexp * sexp = dynamic_cast<const Sexp*>(&cell);

  if(atom)
       stream << *atom;
 
  if(sexp)
       stream << *sexp;

  return stream;
}

std::ostream& operator<< (std::ostream& stream, const Sexp& cell)
{
  stream << "(";
  std::for_each(cell.cells.begin(), cell.cells.end(), [&](std::shared_ptr<Cell> cell){stream << *cell << " ";}); 
  stream << ")";

  return stream;
}

std::ostream& operator<< (std::ostream& stream, const Atom& atom)
{
  stream << atom.val;
  return stream;
}

void Atom::computeType(const std::string& code)
{
     this->type = Atom::Symbol;
     if(isdigit(code.front()) ||  isoperator(code.front()))
          this->type = Atom::Real;
     if(code.front() == '"' &&  code.back() == '"')
          this->type = Atom::String;
}

void Atom::computeVal(const std::string& code) const
{
  this->val = code;
}


std::shared_ptr<Cell> Atom::eval(CellEnv& env)
{
  if(this->type == Atom::Symbol &&  env.find(this->val))
      return env[this->val]->eval(env);
  else if(this->type == Atom::String)
    {
      std::shared_ptr<Cell> res(new Atom());
      *res = *this;
      res->val = this->val.substr(1, this->val.size()-2); 
      return res;
    }
  else
    {
      std::shared_ptr<Cell> res(new Atom());
      *res = *this;
      return res;
    }
}

std::shared_ptr<Cell> Sexp::eval(CellEnv& env)
{
  std::shared_ptr<Cell> cl = this->cells[0];

  if(env.evalHandlers.find(cl->val) != env.evalHandlers.end())
    return env.evalHandlers[cl->val](this, env);


   if(cl->val.compare("setq") == 0)   
    {
      if(!env.find(cells[1]->val))
        env[cells[1]->val].reset(new Atom());
      
      env[cells[1]->val] = cells[2]->eval(env);
      
      return env[cells[1]->val];
    }

  if(cl->val.compare("list") == 0)   
    {
      Sexp * sexp = new Sexp();
      std::shared_ptr<Cell> res(sexp);
      
      std::for_each(cells.begin()+1, cells.end(), [&](std::shared_ptr<Cell> cell){sexp->cells.push_back(cell->eval(env));});

      return res;
    }

  if(cl->val.compare("let") == 0)   
    {
      std::shared_ptr<Sexp> vars = std::dynamic_pointer_cast<Sexp>(this->cells[1]);
      std::shared_ptr<Sexp> body = std::dynamic_pointer_cast<Sexp>(this->cells[2]);

      std::map<std::string, std::shared_ptr<Cell> >* newEnv = new std::map<std::string, std::shared_ptr<Cell> >();
      for(int vId = 0 ; vId < vars->cells.size() ; vId++) {
	std::shared_ptr<Sexp> binding = std::dynamic_pointer_cast<Sexp>(vars->cells[vId]);
	std::shared_ptr<Atom> label = std::dynamic_pointer_cast<Atom>(binding->cells[0]);
	std::shared_ptr<Atom> value = std::dynamic_pointer_cast<Atom>(binding->cells[1]);

	(*newEnv)[label->val].reset(new Atom());
	
	const std::shared_ptr<Cell> res =  value->eval(env);
	(*newEnv)[label->val] = res;
      }

      env.addEnvMap(newEnv);
      std::shared_ptr<Cell> res = body->eval(env);
      env.removeEnv();
      return res;
    }

  if(cl->val.compare("defun") == 0)   
    {
      std::shared_ptr<Atom> fname = std::dynamic_pointer_cast<Atom>(this->cells[1]); //weak
      std::shared_ptr<Sexp> args = std::dynamic_pointer_cast<Sexp>(this->cells[2]); //weak
      std::shared_ptr<Sexp> body = std::dynamic_pointer_cast<Sexp>(this->cells[3]); //weak
      
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
    }
  
  if(cl->val.compare("defmacro") == 0)   
    {
      std::shared_ptr<Atom> fname = std::dynamic_pointer_cast<Atom>(this->cells[1]); //weak
      std::shared_ptr<Sexp> args = std::dynamic_pointer_cast<Sexp>(this->cells[2]); //weak
      std::shared_ptr<Cell> body = this->cells[3];
      
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
    }


  if(cl->val.compare("printenv") == 0)   
    {
      
    }

  
  if(cl->val.compare("lambda") == 0)
    {
      std::shared_ptr<Atom> lambda = std::dynamic_pointer_cast<Atom>(cl);
      std::shared_ptr<Sexp> args =  std::dynamic_pointer_cast<Sexp>(this->cells[1]);
      std::shared_ptr<Sexp> body =  std::dynamic_pointer_cast<Sexp>(this->cells[2]);
      
      if(args && body) {
	this->closure = [env,body, args](Cell* self, std::vector<std::shared_ptr<Cell> > cls) mutable {
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
      
      
      std::shared_ptr<Cell> res(new Atom());
      res->val = "closure";
      res->closure = this->closure;

      return res;
    }
  
  if(cl->val.compare("funcall") == 0) {
    std::shared_ptr<Cell> lambda =  std::dynamic_pointer_cast<Cell>(this->cells[1])->eval(env);
    
    return  lambda->closure(lambda.get(), std::vector<std::shared_ptr<Cell> > (this->cells.begin()+2, this->cells.end()));
    }


  if(env.find(cl->val))
    return  env[cl->val]->closure(this, std::vector<std::shared_ptr<Cell> >(this->cells.begin()+1, this->cells.end()));

  return std::shared_ptr<Cell>(new Atom());  
}

