#include "environment.h"
#include "cell.h"

std::shared_ptr<Cell> parse(std::istream& ss, Cell::CellEnv& env)
{
     bool newToken = false;

     std::vector<std::shared_ptr<Sexp> > sexps;
     std::shared_ptr<Sexp> sexp;
     
     int count = 0;
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
		 count++;
                 std::shared_ptr<Sexp> sx= Sexp::New();
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
                 Atom::Type type = Atom::computeType(buffer);
                 std::shared_ptr<Atom> at;;
                 if(type == Atom::Symbol)
		   {
		     if(sexp)
		       {
			 if(sexp->cells.size() == 0 && quoting == Cell::NoneQ)
			    at = SymbolAtom::New(env, buffer);
			 else
			   at = SymbolAtom::New();
		       }
		     else
		       at = SymbolAtom::New();
		   }
		 if(type == Atom::String)
                   at = StringAtom::New();
                 if(type == Atom::Real)
                   at = RealAtom::New();
                 
		 at->computeVal(buffer);
		 if(!sexps.size())
		   return at;
	
		 if(quoting != Cell::NoneQ)
		   {
		     std::shared_ptr<Sexp> sx = Sexp::New();
		     std::shared_ptr<Atom> quote = SymbolAtom::New();
		     //quote->computeType(quoting == Cell::Quote ? "quote" : "backquote");
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
		 count--;
		 if(sexp->quoting != Cell::NoneQ)
		   {
		     std::shared_ptr<Sexp> sx = Sexp::New();
		     std::shared_ptr<Atom> quote =SymbolAtom::New();
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
      
          if(isalnum(ch) || isoperator(ch) || isquote(ch) || ch == '.')
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
     
     if(count == 0)
       return sexp;
     else
       return std::shared_ptr<Cell>(NULL);
}

bool evalHelper(const std::string& code, Cell::CellEnv& env)
{
  std::stringstream ss;
  ss << code << "\n";
  std::shared_ptr<Cell> sexp = parse(ss, env);
  if(sexp)
    {
      std::cout << "> " << *sexp << std::endl;
      std::cout << "-> " << *sexp->eval(env) << std::endl;
      return true;
    }
  return false;
}

void loadFile(const std::string& file, Cell::CellEnv& env)
{
  std::ifstream in(file);
  while(!in.eof())
    {                              
      std::shared_ptr<Cell> sexp = parse(in, env);
      if(sexp)
	{
          std::cout << "> " << *sexp << std::endl;
	  std::cout << "-> " << *sexp->eval(env) << std::endl;
	}
      else
	break;
    }
}
int main(int argc, char* argv[])
{ 
  Cell::CellEnv env; 
  std::string in;
  std::string curLine;

  Sexp::initGC();
  RealAtom::initGC();
  StringAtom::initGC();
  SymbolAtom::initGC();

  evalHelper("(load \"./core.so\" \"registerCoreHandlers\")", env);
  evalHelper("(load \"./functional.so\" \"registerFunctionalHandlers\")", env);
  evalHelper("(load \"./string.so\" \"registerStringHandlers\")", env);
  evalHelper("(load \"./bench.so\" \"registerBenchHandlers\")", env);
  evalHelper("(load \"./compiler.so\" \"registerCompilerHandlers\")", env);

  if(argc == 2)
    loadFile(argv[1], env);

  while(true)
    {
      std::cout << (curLine.size() ? "     > " : "elisp> ");
      std::getline(std::cin, in);
      if(std::cin.eof())
        {
          std::cout << "goobye" << std::endl;
          break;
        }
      curLine += " " +  in;
      if(evalHelper(curLine, env))
        curLine = "";
    }
  
  return 0;
}
