#include "environment.h"
#include "cell.h"
#include "string.h"
#include "core.h"


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
  registerStringHandlers();
  registerCoreHandlers();

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
