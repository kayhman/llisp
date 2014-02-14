#include <iostream>
#include <list>
#include <vector>
#include <locale>

struct Cell
{
  virtual ~Cell() {};
};

struct Sexp : public Cell
{
  std::vector<Cell*> cells;

  void print(const std::string& code);
};

struct Atom : public Cell
{
  int begin, end;
  void print(const std::string& code);
};

void Atom::print(const std::string& code)
{
  for(int c = this->begin ; c < this->end ; ++c)
    std::cout << code[c];
  std::cout << " ";
}

void Sexp::print(const std::string& code)
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

void parse(const std::string& code)
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
      else if(isalnum(code[c]) && newToken)
	{
	  newToken = false;
	  curTok.begin = c;
	}
    }
  sexp->print(code);

}

int main(int argc, char* argv[])
{
  parse("(lapin 2 (add 3 2))");
  std::cout << std::endl;
  parse("(defun lapin (a b) (add a b))");
  std::cout << std::endl;
  return 0;
}
