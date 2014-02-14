#include <iostream>
#include <list>
#include <vector>
#include <locale>

struct Cell
{
};

struct Sexp : public Cell
{
  std::vector<Cell*> cells;
};

struct Atom : public Cell
{
  int begin, end;
};

void parse(const std::string& code)
{
  bool newToken = false;
  Atom curTok;
  std::vector<Atom> tokens;
  curTok.begin = -1;

  Sexp sexp;

  for(int c = 0 ; c < code.size() ; ++c)
    {
      std::cout << code[c];
      if(code[c] == '(' ||
	 code[c] == ' ' ||
	 code[c] == ')')
	{
	  if(curTok.begin == -1)
	    newToken = true;
	  else
	    {
	      newToken = true;
	      curTok.end = c;
	      tokens.push_back(curTok);
	      
	      Atom* at = new Atom();
	      *at = curTok;
	      
	      sexp.cells.push_back(at);

	      curTok.begin =  -1;
	    }
	}
      else if(isalnum(code[c]) && newToken)
	{
	  newToken = false;
	  curTok.begin = c;
	}
	
    }
  std::cout << std::endl;
  
  std::cout << "#tokens " << tokens.size() << std::endl;

  for(int tId = 0 ; tId < tokens.size() ; ++tId)
    {
      for(int c = tokens[tId].begin ; c < tokens[tId].end ; ++c)
	std::cout << code[c];
      std::cout << std::endl;
    }

}

int main(int argc, char* argv[])
{
  parse("(lapin 2 3)");
  return 0;
}
