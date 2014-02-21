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

bool isoperator(char c) {
     std::string const valid_chars = "+*-/!=<>\"'`";
     return valid_chars.find(c) != std::string::npos;
}


struct Cell
{
  virtual ~Cell() {};
  mutable std::function<std::string(std::vector<std::shared_ptr<Cell> >)> closure;
  mutable std::string val;
  
  virtual void print() const = 0;
  virtual std::string eval() const = 0;
};

template <typename Key, typename Val>
class Env
{
private:
  std::vector<std::map<Key,Val> > envs;
public:
  Val& operator[] (const Key& k);
  Val& operator[] (Key& k);
  bool find (const Key& k);
};

template <typename Key, typename Val>
Val& Env<Key, Val>::operator[] (const Key& k)
{
  for(auto envIt = envs.rbegin() ; envIt != envs.rend() ; envIt++)
    if(envIt->find(k) != envIt->end())
      return (*envIt)[k];
}

template <typename Key, typename Val>
Val& Env<Key, Val>::operator[] (Key& k)
{
  for(auto envIt = envs.rbegin() ; envIt != envs.rend() ; envIt++)
    if(envIt->find(k) != envIt->end())
      return (*envIt)[k];
}

template <typename Key, typename Val>
bool Env<Key, Val>::find (const Key& k)
{
  for(auto envIt = envs.rbegin() ; envIt != envs.rend() ; envIt++)
    if(envIt->find(k) != envIt->end())
      return true;

  return false;
}

template class Env<std::string, std::shared_ptr<Cell> >;

//std::map<std::string, std::shared_ptr<Cell> > env;
Env<std::string, std::shared_ptr<Cell> > env;

struct Sexp : public Cell
{
  std::vector<std::shared_ptr<Cell> > cells;
  
  void print() const;
  virtual std::string eval() const;
};


struct Atom : public Cell
{
  enum Type {Symbol, Real, String, Closure};
  Type type;
  
  void print() const;
  void computeType(const std::string& code);
  void computeVal(const std::string& code) const;
  virtual std::string eval() const;
};

void Atom::print() const
{
     if(this->type == Atom::Real)
          std::cout << "i'";
     else if (this->type == Atom::String)
          std::cout << "S'";
     else
          std::cout << "s'";
     std::cout << this->val;
     std::cout << " ";
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


std::string Atom::eval() const
{
  if(this->type == Atom::Symbol &&  env.find(this->val) != env.end() )
    {
      const std::string& res = env[this->val]->eval();
      this->closure = env[this->val]->closure;
      return res;
    }
  else if(this->type == Atom::String)
    return this->val.substr(1, this->val.size()-2);
  else if(this->val.front() == '\'')
    return this->val.substr(1, this->val.size()-1);
  else
    return this->val;
}

void Sexp::print() const
{
     std::cout << "[";
     for(int cId = 0 ; cId < cells.size() ; ++cId)
     {
       std::shared_ptr<Cell> cl = this->cells[cId]; //weak
       std::shared_ptr<Atom> at = std::dynamic_pointer_cast<Atom>(cl); //weak
       std::shared_ptr<Sexp> sx = std::dynamic_pointer_cast<Sexp>(cl); //weak
      
          if(at)
               at->print();
          if(sx)
               sx->print();
     }
     std::cout << "]"<< std::endl;
}

std::string Sexp::eval() const
{
  std::shared_ptr<Cell> cl = this->cells[0];

  if(cl->val.compare("concat") == 0)
    std::for_each(cells.begin()+1, cells.end(), [&](std::shared_ptr<Cell> cell){this->val += cell->eval();}); 
  
  if(cl->val.compare("+") == 0)
    {
      double res = 0;
      std::for_each(cells.begin()+1, cells.end(), [&](std::shared_ptr<Cell> cell){res += atof(cell->eval().c_str());});
      std::ostringstream ss;
      ss << res;
      return ss.str(); 
    }
  
  if(cl->val.compare("-") == 0)
    {
      double res = atof(cells[1]->eval().c_str());
      std::for_each(cells.begin()+2, cells.end(), [&](std::shared_ptr<Cell> cell){res -= atof(cell->eval().c_str());});
      std::ostringstream ss;
      ss << res;
      return ss.str(); 
    }
  
  if(cl->val.compare("*") == 0)
    {
      double res = 1.0;
      std::for_each(cells.begin()+1, cells.end(), [&](std::shared_ptr<Cell> cell){res *= atof(cell->eval().c_str());});
      std::ostringstream ss;
      ss << res;
      return ss.str(); 
    }
  
  if(cl->val.compare("/") == 0)
    {
      double res = atof(cells[1]->eval().c_str());
      std::for_each(cells.begin()+2, cells.end(), [&](std::shared_ptr<Cell> cell){res /= atof(cell->eval().c_str());});
      std::ostringstream ss;
      ss << res;
      return ss.str(); 
    }

  if(cl->val.compare("progn") == 0)
    {
      std::for_each(cells.begin()+1, cells.end()-1, [&](std::shared_ptr<Cell> cell){cell->eval();});
      return cells.back()->eval(); 
    }
  
  if(cl->val.compare("setq") == 0)   
    {
      if(env.find(cells[1]->val) == env.end())
        env[cells[1]->val].reset(new Atom());
      
      const std::string& res =  cells[2]->eval();
      env[cells[1]->val]->val = res;
      env[cells[1]->val]->closure = cells[2]->closure;
              
      return res;
    }
  
  if(cl->val.compare("defun") == 0)   
    {
      std::shared_ptr<Atom> fname = std::dynamic_pointer_cast<Atom>(this->cells[1]); //weak
      std::shared_ptr<Sexp> args = std::dynamic_pointer_cast<Sexp>(this->cells[2]); //weak
      std::shared_ptr<Sexp> body = std::dynamic_pointer_cast<Sexp>(this->cells[3]); //weak
      
      if(args && body)
        {
          fname->closure = [args, body, fname](std::vector<std::shared_ptr<Cell> > cls) {

            //assert(cls.size() == args->cells.size());
            for(int c = 0 ; c < cls.size() ; c++)
              env[args->cells[c]->val] = cls[c];

            std::string res = body->eval();
            //clean env
             for(int c = 0 ; c < cls.size() ; c++)
               env.erase(args->cells[c]->val);

            return res;
          };
        }
      env[fname->val] = fname;
      return "Defined " + fname->val;
    }

 if(cl->val.compare("printenv") == 0)   
    {
      std::stringstream ss;
      ss << std::endl;
      std::for_each(env.begin(), env.end(), [&](std::pair<std::string, std::shared_ptr<Cell> > p){ss << p.first << " -> " << p.second->val << std::endl;});
      return ss.str();
    }

  
  if(cl->val.compare("lambda") == 0)
    {
      std::shared_ptr<Atom> lambda = std::dynamic_pointer_cast<Atom>(cl);
      std::shared_ptr<Sexp> args =  std::dynamic_pointer_cast<Sexp>(this->cells[1]);
      std::shared_ptr<Sexp> body =  std::dynamic_pointer_cast<Sexp>(this->cells[2]);
      
      if(args && body) {
          this->closure = [body, args](std::vector<std::shared_ptr<Cell> > cls) {
            
            //assert(cls.size() == args->cells.size());
            for(int c = 0 ; c < cls.size() ; c++)
              env[args->cells[c]->val] = cls[c];
            
            std::string res = body->eval();
            
            //clean env
             for(int c = 0 ; c < cls.size() ; c++)
               env.erase(args->cells[c]->val);
            
            return res;
          };
        }
      
      return "closure";
    }
  
  if(cl->val.compare("funcall") == 0) {
      std::shared_ptr<Cell> lambda =  std::dynamic_pointer_cast<Cell>(this->cells[1]);
      lambda->eval();
      return  lambda->closure(std::vector<std::shared_ptr<Cell> > (this->cells.begin()+2, this->cells.end()));
    }


  if(env.find(cl->val) != env.end())
    return  env[cl->val]->closure( std::vector<std::shared_ptr<Cell> >(this->cells.begin()+1, this->cells.end()));

  
  return this->val;
  
}

std::shared_ptr<Sexp> parse(std::istream& ss)
{
     bool newToken = false;
     Atom curTok;

     std::vector<std::shared_ptr<Sexp> > sexps;
     std::shared_ptr<Sexp> sexp;
  
     char ch;
     std::string buffer;
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
               }


               newToken = true;
	  
               if(buffer.size())
               {
                    curTok.computeType(buffer);
                    curTok.computeVal(buffer);
	      
                    std::shared_ptr<Atom> at(new Atom());
                    *at = curTok;
	      
                    sexp->cells.push_back(at);

                    buffer.resize(0);
               }

               if(ch == ')')
               {
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
          }
          else if((isalnum(ch) || isoperator(ch)) && newToken)
          {
               newToken = false;
          }
      
          if(isalnum(ch) || isoperator(ch) || ch == '.')
          {
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
     
    while(!in.eof())
     {                              
       std::shared_ptr<Sexp> sexp = parse(in);
       if(sexp)
         {
           sexp->print();
           std::cout << "-> " << sexp->eval() << std::endl;
           std::cout << "check " << sexp->cells.size() << std::endl;
         }
          else
               break;
     }
     
     return 0;
}
