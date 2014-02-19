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
     mutable std::string val;
     mutable std::function<Cell*(std::vector<Cell*>)> closure;
     virtual void print() const = 0;
     virtual void eval() const = 0;
};

std::map<std::string, Cell*> env;

struct Sexp : public Cell
{
     std::vector<Cell*> cells;

     void print() const;
     virtual void eval() const;
};


struct Atom : public Cell
{
     enum Type {Symbol, Real, String, Closure};
     Type type;

     void print() const;
     void computeType(const std::string& code);
     void computeVal(const std::string& code) const;
     virtual void eval() const;
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
     this-> type = Atom::Symbol;
     if(isdigit(code.front()) ||  isoperator(code.front()))
          this-> type = Atom::Real;
     if(code.front() == '"' &&  code.back() == '"')
          this-> type = Atom::String;
}

void Atom::computeVal(const std::string& code) const
{
     if(this->type == Atom::String)
          this->val = code.substr(1, code.size()-2);
     else
          this->val = code;
}


void Atom::eval() const
{
     if(this->type == Atom::Symbol && 
        env.find(this->val) != env.end() )
          this->val = env[this->val]->val; //TODO : we overwrite value -> bug
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
     std::cout << "]";
}

void Sexp::eval() const
{
     Cell* cl = this->cells[0];
     cl->eval();

     if(cl->val.compare("concat") == 0)
     {
          std::for_each(cells.begin()+1, cells.end(), [&](Cell* cell){cell->eval();}); 
          std::for_each(cells.begin()+1, cells.end(), [&](Cell* cell){this->val += cell->val;}); 
     }

     if(cl->val.compare("+") == 0)
     {
          std::for_each(cells.begin()+1, cells.end(), [&](Cell* cell){cell->eval();});
          double res = 0;
          std::for_each(cells.begin()+1, cells.end(), [&](Cell* cell){res += atof(cell->val.c_str());});
          std::ostringstream ss;
          ss << res;
          this->val = ss.str(); 
     }

     if(cl->val.compare("define") == 0)   
     {
          Sexp* lambda = dynamic_cast<Sexp*>(cells[2]);
          std::for_each(cells.begin()+1, cells.end(), [&](Cell* cell){cell->eval();});
          
          if(lambda)
               if(lambda->cells.size() > 0)
                    if(lambda->cells[0]->val.compare("lambda") == 0)
                         cells[2]->val = cells[1]->val;
          env[cells[1]->val] = cells[2];
     }
     
     if(cl->val.compare("lambda") == 0)
     {
          this->cells[1]->eval();
          Sexp* args = dynamic_cast<Sexp*>(this->cells[1]);
          Sexp* body = dynamic_cast<Sexp*>(this->cells[2]);
          if(args && body)
          {
               this->closure = [&](std::vector<Cell*> cls) {
                    Sexp* args = dynamic_cast<Sexp*>(this->cells[1]);
                    Sexp* body = dynamic_cast<Sexp*>(this->cells[2]);
            
                    std::for_each(cls.begin(), cls.end(), [&](Cell* cell){cell->eval();});

                    //assert(cls.size() == args->cells.size());
                    for(int c = 0 ; c < cls.size() ; c++)
                         env[args->cells[c]->val] = cls[c];
                    body->eval();

                    //clean env
                    for(int c = 0 ; c < cls.size() ; c++)
                         env.erase(args->cells[c]->val);
            
                    return body;
               };
          }
     }
  
     if(env.find(cl->val) != env.end())
     {
          std::vector<Cell*> args(this->cells.begin()+1, this->cells.end());
          Cell * res = env[cl->val]->closure(args);
          this->val = res->val;
     }

     std::cout << " -> " << this->val << std::endl;


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
               sexp->eval();
          }
          else
               break;
     }
     
     return 0;
/*
     std::string code = "(define boubou \"chaise\")";
     Sexp* sexp = parse(code);
     sexp->print();
     std::cout << std::endl;
     sexp->eval();
  
     code = "(defun lapin (a b) (add a b \"c\"))";
     sexp = parse(code);
     sexp->print();
     std::cout << std::endl;
     std::cout << std::endl;
     std::cout << std::endl;


     code = "(concat \"one\" \"two\" (concat boubou \"pin\" \"pin\"))";
     sexp = parse(code);
     sexp->print();
     std::cout << std::endl;
     sexp->eval();
     std::cout << "eval : " << sexp->val << std::endl;

     //create var
     code = "(define lapin 12.666)";
     sexp = parse(code);
     sexp->print();
     std::cout << std::endl;
     sexp->eval();
     std::cout << "eval : " << sexp->val << std::endl;


     code = "(+ 1 2 (+ lapin 3.666))";
     sexp = parse(code);
     sexp->print();
     std::cout << std::endl;
     sexp->eval();
     std::cout << "eval : " << sexp->val << std::endl;



     code = "(define try (lambda (a b) (+ a b)))";
     sexp = parse(code);
     sexp->print();
     std::cout << std::endl;
     sexp->eval();
     std::cout << "eval : " << sexp->val << std::endl;


     code = "(try 0.777 0.666)";
     sexp = parse(code);
     sexp->print();
     std::cout << std::endl;
     sexp->eval();
     std::cout << "eval : " << sexp->val << std::endl;

     code = "(define myconcat (lambda (a b) (concat a b)))";
     sexp = parse(code);
     sexp->print();
     std::cout << std::endl;
     sexp->eval();
     std::cout << "eval : " << sexp->val << std::endl;


     code = "(myconcat \"jack\" \"paulo\")";
     sexp = parse(code);
     sexp->print();
     std::cout << std::endl;
     sexp->eval();
     std::cout << "eval : " << sexp->val << std::endl;

     return 0;
*/
}
