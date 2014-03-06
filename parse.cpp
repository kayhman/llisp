#include <iostream>
#include <list>
#include <vector>
#include <locale>
#include <numeric>
#include <algorithm>
#include <sstream>
#include <map>
#include <functional>
#include <fstream>
#include <memory>
#include <regex>

bool isoperator(char c) {
     std::string const valid_chars = "+*-/!=<>\"'`,";
     return valid_chars.find(c) != std::string::npos;
}



template <typename Key, typename Val>
class Env
{
public:
  static std::map<Key,Val> top;
private:
  std::vector<std::map<Key,Val>* > envs;
public:
  Val& operator[] (const Key& k);
  Val& operator[] (Key& k);
  bool find (const Key& k);
  Env();
  void addEnvMap(std::map<Key, Val>* env);
  void removeEnv();
};

struct Cell
{
  enum Quoting {Quote, BackQuote, NoneQ};
  typedef Env<std::string, std::shared_ptr<Cell> > CellEnv;
  virtual ~Cell() {};
  Cell() :quoting(NoneQ) {};
  mutable std::function<std::shared_ptr<Cell>(Cell* self, std::vector<std::shared_ptr<Cell> >)> closure;
  mutable std::string val;
  Quoting quoting;

  virtual std::shared_ptr<Cell> eval(CellEnv& env) = 0;

  friend std::ostream& operator<< (std::ostream& stream, const Cell& cell);
};

template<typename K, typename V> std::map<K, V> Env<K,V>::top;

template <typename Key, typename Val>
Env<Key, Val>::Env()
{
  envs.push_back(&top);
}

template <typename Key, typename Val>
void Env<Key, Val>::addEnvMap(std::map<Key, Val>* env)
{
  this->envs.push_back(env);
}

template <typename Key, typename Val>
void Env<Key, Val>::removeEnv()
{
  this->envs.pop_back();
}

template <typename Key, typename Val>
Val& Env<Key, Val>::operator[] (const Key& k)
{
  for(auto envIt = envs.rbegin() ; envIt != envs.rend() ; envIt++)
    if((*envIt)->find(k) != (*envIt)->end())
 	return (**envIt)[k];
   return (*envs.back())[k];
}

template <typename Key, typename Val>
Val& Env<Key, Val>::operator[] (Key& k)
{
  for(auto envIt = envs.rbegin() ; envIt != envs.rend() ; envIt++)
    if((*envIt)->find(k) != (*envIt)->end())
 	return (**envIt)[k];
   return (*envs.back())[k];
}

template <typename Key, typename Val>
bool Env<Key, Val>::find (const Key& k)
{
  for(auto envIt = envs.rbegin() ; envIt != envs.rend() ; envIt++)
    if((*envIt)->find(k) != (*envIt)->end())
	return true;
  return false;
}

struct Sexp : public Cell
{
  std::vector<std::shared_ptr<Cell> > cells;
  
  virtual std::shared_ptr<Cell> eval(CellEnv& env);

  friend std::ostream& operator<< (std::ostream& stream, const Sexp& cell);
};


struct Atom : public Cell
{
  enum Type {Symbol, Real, String, Closure};
  Type type;
  
  void computeType(const std::string& code);
  void computeVal(const std::string& code) const;
  virtual std::shared_ptr<Cell> eval(CellEnv& env);

  friend std::ostream& operator<< (std::ostream& stream, const Atom& cell);
};


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

  if(cl->val.compare("concat") == 0)
    {
      std::shared_ptr<Cell> res(new Atom());
      std::for_each(cells.begin()+1, cells.end(), [&](std::shared_ptr<Cell> cell){res->val += cell->eval(env)->val;}); 
      return res;
    }

  
  if(cl->val.compare("append") == 0)
    {
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
    }

  if(cl->val.compare("+") == 0)
    {
      std::shared_ptr<Cell> res(new Atom());
      double sum = 0;
      std::for_each(cells.begin()+1, cells.end(), [&](std::shared_ptr<Cell> cell){sum += atof(cell->eval(env)->val.c_str());});
      std::ostringstream ss;
      ss << sum;
      res->val = ss.str();
      return res;
    }
  
  if(cl->val.compare("-") == 0)
    {
      std::shared_ptr<Cell> res(new Atom());
      double sum = atof(cells[1]->eval(env)->val.c_str());
      std::for_each(cells.begin()+2, cells.end(), [&](std::shared_ptr<Cell> cell){sum -= atof(cell->eval(env)->val.c_str());});

      std::ostringstream ss;
      ss << sum;
      res->val = ss.str();
      return res;
    }
  
  if(cl->val.compare("*") == 0)
    {
      std::shared_ptr<Cell> res(new Atom());
      double prod = 1.0;
      std::for_each(cells.begin()+1, cells.end(), [&](std::shared_ptr<Cell> cell){prod *= atof(cell->eval(env)->val.c_str());});
      std::ostringstream ss;
      ss << prod;
      res->val = ss.str();
      return res;
    }
  
  if(cl->val.compare("/") == 0)
    {
      std::shared_ptr<Cell> res(new Atom());
      double quotient = atof(cells[1]->eval(env)->val.c_str());
      std::for_each(cells.begin()+2, cells.end(), [&](std::shared_ptr<Cell> cell){quotient /= atof(cell->eval(env)->val.c_str());});
      std::ostringstream ss;
      ss << quotient;
      res->val = ss.str();
      return res;
    }

  if(cl->val.compare("eq") == 0)
    {
      std::shared_ptr<Cell> m1 = this->cells[1];
      std::shared_ptr<Cell> m2 = this->cells[2];
      
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
    }


  if(cl->val.compare("if") == 0)
    {
      std::shared_ptr<Cell> test = this->cells[1];
      std::shared_ptr<Cell> True = this->cells[2];
      std::shared_ptr<Cell> False = this->cells[3];

      if(test->eval(env)->val != "")
        return True->eval(env);
      else        
        return False->eval(env);
    }

  if(cl->val.compare("quote") == 0 ||
     cl->val.compare("backquote") == 0 )
    {
      return cells[1];
    }


  if(cl->val.compare("progn") == 0)
    {
      std::for_each(cells.begin()+1, cells.end()-1, [&](std::shared_ptr<Cell> cell){cell->eval(env);});
      return cells.back()->eval(env); 
    }
  
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

std::shared_ptr<Sexp> parse(std::istream& ss)
{
     bool newToken = false;
     Atom curTok;

     std::vector<std::shared_ptr<Sexp> > sexps;
     std::shared_ptr<Sexp> sexp;
  
     char ch;
     std::string buffer;
     Cell::Quoting quoting = Cell::NoneQ;
     for(int cc = 0 ;  ; ++cc)
     {
          ss >> std::noskipws >> ch;
          if(ch == '(' || ch == ' ' || ch == ')' || ch == '\n')
          {
               if(ch == '(')
               {
                 std::shared_ptr<Sexp> sx(new Sexp());
                 if(sexps.size())
                   sexp->cells.push_back(sx);
                 sexp = sx;
                 sexps.push_back(sx);
		 if(quoting == Cell::Quote)
		   sexp->quoting = Cell::Quote;
		 if(quoting == Cell::BackQuote)
		   sexp->quoting = Cell::BackQuote;
		 quoting = Cell::NoneQ;
	       }

               newToken = true;
	  
               if(buffer.size())
               {
                    curTok.computeType(buffer);
                    curTok.computeVal(buffer);
	      
                    std::shared_ptr<Atom> at(new Atom());
                    *at = curTok;
		    
		    if(quoting != Cell::NoneQ)
		      {
			std::shared_ptr<Sexp> sx(new Sexp());
			std::shared_ptr<Atom> quote(new Atom());
			quote->computeType(quoting == Cell::Quote ? "quote" : "backquote");
			quote->computeVal(quoting == Cell::Quote ? "quote" : "backquote");
		    
			sx->cells.push_back(quote);
			sx->cells.push_back(at);
			sexp->cells.push_back(sx);
			quoting = Cell::NoneQ;
		      }
		    else
		      sexp->cells.push_back(at);

                    buffer.resize(0);
               }

               if(ch == ')')
               {
		 if(sexp->quoting != Cell::NoneQ)
		   {
		     std::shared_ptr<Sexp> sx(new Sexp());
		     std::shared_ptr<Atom> quote(new Atom());
		     quote->computeType(sexp->quoting == Cell::Quote ? "quote" : "backquote");
		     quote->computeVal(sexp->quoting == Cell::Quote ? "quote" : "backquote");

		     sx->cells = sexps.back()->cells;
		     sexps.back()->cells.resize(0);
		     sexps.back()->cells.push_back(quote);
		     sexps.back()->cells.push_back(sx);
		   }


                    if(sexps.size() > 1)
                    {
                         sexps.pop_back();
                         sexp = sexps.back();
                    }
                    else
                    {
                         sexp = sexps.back();
                         sexps.pop_back();
                    }
               }
	       quoting = Cell::NoneQ;
          }
          else if((isalnum(ch) || isoperator(ch)) && newToken)
          {
               newToken = false;
          }
      
          if(isalnum(ch) || isoperator(ch) || ch == '.')
          {
            if(ch == '\'')
	      quoting = Cell::Quote;
	    else if(ch == '`')
              quoting = Cell::BackQuote;
            else
              buffer.push_back(ch);
          }

          if((sexps.size() == 0 && sexp != NULL) 
             || ss.eof())
               break; //done with parsing
     }
  
     return sexp;
}

int main(int argc, char* argv[])
{
  std::ifstream in(argv[1]);
  Cell::CellEnv env;     
  while(!in.eof())
    {                              
      std::shared_ptr<Sexp> sexp = parse(in);
      if(sexp)
	{
          std::cout << "> " << *sexp << std::endl;
	  std::cout << "-> " << *sexp->eval(env) << std::endl;
	}
      else
	break;
    }
  
  return 0;
}
