#include <iostream>
#include <list>
#include <vector>
#include <locale>
#include <numeric>
#include <algorithm>
#include <sstream>

bool isoperator(char c) {
    std::string const valid_chars = "+*-/!=<>\"";
    return valid_chars.find(c) != std::string::npos;
}


struct Cell
{
  virtual ~Cell() {};
  mutable std::string val;
  virtual void eval(const std::string& code) const = 0;
};

struct Sexp : public Cell
{
  std::vector<Cell*> cells;

  void print(const std::string& code) const;
  virtual void eval(const std::string& code) const;
};


struct Atom : public Cell
{
  enum Type {Symbol, Real, String};
  Type type;
  int begin, end;
  void print(const std::string& code) const;
  void computeType(const std::string& code);
  virtual void eval(const std::string& code) const;
};

void Atom::print(const std::string& code) const
{
  if(this->type == Atom::Real)
    std::cout << "i'";
  else if (this->type == Atom::String)
    std::cout << "S'";
  else
    std::cout << "s'";
  for(int c = this->begin ; c < this->end ; ++c)
    std::cout << code[c];
  std::cout << " ";
}

void Atom::computeType(const std::string& code)
{
  if(this->begin > 0 && this->begin < code.size() &&
     this->end > 0 && this->end < code.size())
    {
      this->val = code.substr(this->begin, this->end);
      this-> type = Atom::Symbol;
      if(isdigit(code[this->begin]) ||  isoperator(code[this->begin]))
	this-> type = Atom::Real;
      if(code[this->begin] == '"' &&  code[this->end-1] == '"')
	this-> type = Atom::String;
    }
}
void Atom::eval(const std::string& code) const
{
  if(this->type == Atom::String)
    this->val = code.substr(this->begin+1, this->end-this->begin-2);
  else
    this->val = code.substr(this->begin, this->end-this->begin);
}

void Sexp::print(const std::string& code) const
{
  std::cout << "[";
  for(int cId = 0 ; cId < cells.size() ; ++cId)
    {
      Cell* cl = this->cells[cId];
      Atom* at = dynamic_cast<Atom*>(cl);
      Sexp* sx = dynamic_cast<Sexp*>(cl);
      
      if(at)
	at->print(code);
      if(sx)
	sx->print(code);
    }
  std::cout << "]";
}

void Sexp::eval(const std::string& code) const
{
  Cell* cl = this->cells[0];
  cl->eval(code);

  if(cl->val.compare("concat") == 0)
    {
      std::for_each(cells.begin()+1, cells.end(), [&](Cell* cell){cell->eval(code);}); 
      std::for_each(cells.begin()+1, cells.end(), [&](Cell* cell){this->val += cell->val;}); 
    }

  if(cl->val.compare("add") == 0)
    {
      std::for_each(cells.begin()+1, cells.end(), [&](Cell* cell){cell->eval(code);});
      double res = 0;
      std::for_each(cells.begin()+1, cells.end(), [&](Cell* cell){res += atof(cell->val.c_str());});
      std::ostringstream ss;
      ss << res;
      this->val = ss.str(); 
    }
}

Sexp* parse(const std::string& code)
{
  bool newToken = false;
  Atom curTok;
  curTok.begin = -1;

  std::vector<Sexp*> sexps;
  Sexp* sexp = NULL;

  for(int c = 0 ; c < code.size() ; ++c)
    {
      std::cout << code[c];
      if(code[c] == '(' ||
	 code[c] == ' ' ||
	 code[c] == ')')
	{
	  if(code[c] == '(')
	    {
	      Sexp* sx = new Sexp();
	      if(sexp)
		sexp->cells.push_back(sx);
	      sexp = sx;
	      sexps.push_back(sx);
	    }

	  if(curTok.begin == -1)
	    newToken = true;
	  else
	    {
	      newToken = true;
	      curTok.end = c;
	      curTok.computeType(code);

	      Atom* at = new Atom();
	      *at = curTok;
	      
	      sexp->cells.push_back(at);

	      curTok.begin =  -1;
	    }

	  if(code[c] == ')')
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
      else if((isalnum(code[c]) || isoperator(code[c])) && newToken)
	{
	  newToken = false;
	  curTok.begin = c;
	}
    }
  
  return sexp;
}

int main(int argc, char* argv[])
{
  std::string code = "(lapin 2 (add -3 -2))";
  Sexp* sexp = parse(code);
  sexp->print(code);
  std::cout << std::endl;
  
  code = "(defun lapin (a b) (add a b \"c\"))";
  sexp = parse(code);
  sexp->print(code);
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;


  code = "(concat \"one\" \"two\")";
  sexp = parse(code);
  sexp->print(code);
  std::cout << std::endl;
  sexp->eval(code);
  std::cout << "eval : " << sexp->val << std::endl;


  code = "(add 1 2 3.666)";
  sexp = parse(code);
  sexp->print(code);
  std::cout << std::endl;
  sexp->eval(code);
  std::cout << "eval : " << sexp->val << std::endl;
  return 0;
}
