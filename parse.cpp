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

bool isoperator(char c) {
     std::string const valid_chars = "+*-/!=<>\"";
     return valid_chars.find(c) != std::string::npos;
}


struct Cell
{
  virtual ~Cell() {};
  mutable std::function<std::string(std::vector<Cell*>)> closure;
  virtual void print() const = 0;
  virtual std::string eval() const = 0;
  mutable std::string val;
};

std::map<std::string, Cell*> env;

struct Sexp : public Cell
{
  std::vector<Cell*> cells;
  
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
     //this->val = code;
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
    return env[this->val]->eval();
  else if(this->type == Atom::String)
    return this->val.substr(1, this->val.size()-2);
  else
    return this->val;
}

void Sexp::print() const
{
     std::cout << "[";
     for(int cId = 0 ; cId < cells.size() ; ++cId)
     {
          Cell* cl = this->cells[cId];
          Atom* at = dynamic_cast<Atom*>(cl);
          Sexp* sx = dynamic_cast<Sexp*>(cl);
      
          if(at)
               at->print();
          if(sx)
               sx->print();
     }
     std::cout << "]"<< std::endl;
}

std::string Sexp::eval() const
{
  //  std::cout << "size " << this->cells.size() << std::endl;
  
  Cell* cl = this->cells[0];


  if(cl->val.compare("concat") == 0)
    std::for_each(cells.begin()+1, cells.end(), [&](Cell* cell){this->val += cell->eval();}); 
  
  if(cl->val.compare("+") == 0)
    {
      double res = 0;
      std::for_each(cells.begin()+1, cells.end(), [&](Cell* cell){res += atof(cell->eval().c_str());});
      std::ostringstream ss;
      ss << res;
      return ss.str(); 
    }
  
  if(cl->val.compare("-") == 0)
    {
      double res = atof(cells[1]->eval().c_str());
      std::for_each(cells.begin()+2, cells.end(), [&](Cell* cell){res -= atof(cell->eval().c_str());});
      std::ostringstream ss;
      ss << res;
      return ss.str(); 
    }
  
  if(cl->val.compare("*") == 0)
    {
      double res = 1.0;
      std::for_each(cells.begin()+1, cells.end(), [&](Cell* cell){res *= atof(cell->eval().c_str());});
      std::ostringstream ss;
      ss << res;
      return ss.str(); 
    }
  
  if(cl->val.compare("/") == 0)
    {
      double res = atof(cells[1]->eval().c_str());
      std::for_each(cells.begin()+2, cells.end(), [&](Cell* cell){res /= atof(cell->eval().c_str());});
      std::ostringstream ss;
      ss << res;
      return ss.str(); 
    }
  
  if(cl->val.compare("setq") == 0)   
    {
      //std::for_each(cells.begin()+2, cells.end(), [&](Cell* cell){cell->eval();});
      env[cells[1]->val] = cells[2];
      return cells[2]->eval();
    }
  
  if(cl->val.compare("defun") == 0)   
    {
      Atom* fname = dynamic_cast<Atom*>(this->cells[1]);
      Sexp* args = dynamic_cast<Sexp*>(this->cells[2]);
      Sexp* body = dynamic_cast<Sexp*>(this->cells[3]);
      
      if(args && body)
        {
          fname->closure = [&](std::vector<Cell*> cls) {
            Sexp* args = dynamic_cast<Sexp*>(this->cells[2]);
            Sexp* body = dynamic_cast<Sexp*>(this->cells[3]);
            
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
      return "";
    }

  /*
  if(cl->val.compare("lambda") == 0)
    {
      this->cells[1]->eval();
      Sexp* args = dynamic_cast<Sexp*>(this->cells[1]);
      Sexp* body = dynamic_cast<Sexp*>(this->cells[2]);
      if(args && body)
        {
          this->closure = [&](std::vector<Cell*> cls) {
            std::cout << "call func" << std::endl;
            Sexp* args = dynamic_cast<Sexp*>(this->cells[1]);
            Sexp* body = dynamic_cast<Sexp*>(this->cells[2]);
            
            std::for_each(cls.begin(), cls.end(), [&](Cell* cell){cell->eval();});
            
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
      return "";
      }*/


  if(env.find(cl->val) != env.end())
    {

      std::vector<Cell*> args(this->cells.begin()+1, this->cells.end());
      return  env[cl->val]->closure(args);
    }
  
  return this->val;
  
}

Sexp* parse(std::istream& ss)
{
     bool newToken = false;
     Atom curTok;

     std::vector<Sexp*> sexps;
     Sexp* sexp = NULL;
  
     char ch;
     std::string buffer;
     for(int cc = 0 ;  ; ++cc)
     {
          ss >> std::noskipws >> ch;
          if(ch == '(' || ch == ' ' || ch == ')')
          {
               if(ch == '(')
               {
                    Sexp* sx = new Sexp();
                    if(sexp)
                         sexp->cells.push_back(sx);
                    sexp = sx;
                    sexps.push_back(sx);
               }


               newToken = true;
	  
               if(buffer.size())
               {
                    curTok.computeType(buffer);
                    curTok.computeVal(buffer);
	      
                    Atom* at = new Atom();
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
          Sexp* sexp = parse(in);
          if(sexp)
          {
            sexp->print();
            //sexp->eval();
            std::cout << "-> " << sexp->eval() << std::endl;
          }
          else
               break;
     }
     
     return 0;
}
