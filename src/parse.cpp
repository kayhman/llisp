#include "environment.h"
#include "cell.h"
#include <istream>

std::shared_ptr<Cell> parseCell(std::istream& is, Cell::CellEnv& env);

std::shared_ptr<Cell> parseString(std::istream& is, Cell::CellEnv& env) {
  char ch;
  std::string atom;
  std::stringstream ss;
  is >> ch;
  //assert(ch == '"')
  while(is >> std::noskipws >> ch) {
    if(ch == '"')
      break;
    ss << ch;
  }
  is >> std::skipws;
  atom = ss.str();
  Atom::Type type = Atom::computeType(atom);
  //assert(type == Atom::String);
  std::shared_ptr<Cell> at = StringAtom::New();
  std::dynamic_pointer_cast<Atom>(at)->computeVal(atom);
  return at;
}

std::shared_ptr<Cell> parseAtom(std::istream& is, Cell::CellEnv& env) {
  std::shared_ptr<Cell> at;  
  char ch;
  is >> ch;
  is.putback(ch);
  if (ch == '"') {
    at = parseString(is, env);
  } else {
    std::string atom;
    is >> atom;
    Atom::Type type = Atom::computeType(atom);
    
    if(type == Atom::Symbol)
      at = SymbolAtom::New(env, atom);
    if(type == Atom::Real)
      at = RealAtom::New();
    std::dynamic_pointer_cast<Atom>(at)->computeVal(atom);
  }
  

  return at; 
}

std::shared_ptr<Cell> parseSexp(std::istream& is, Cell::CellEnv& env) {
  char ch;
  std::shared_ptr<Sexp> sx = Sexp::New();
  while(is >> ch) {
    if (ch == ')') {
      return sx;
    } else {
      is.putback(ch);
      std::shared_ptr<Cell> cell = parseCell(is, env);
      sx->cells.push_back(cell);
    }
  }
}


std::shared_ptr<Cell> parseCell(std::istream& is, Cell::CellEnv& env) {
  char ch;
  is >> ch;

  std::shared_ptr<Sexp> quote;
  std::shared_ptr<Cell> cell;

  if (ch == '\'' || ch == '`' || ch == ',') {
    quote = Sexp::New();
    std::string atomName = ch == '\'' ? "quote" : (ch == '`' ? "backquote" : "comma"); 
    std::shared_ptr<Atom> atom = SymbolAtom::New(env, atomName);
    atom->computeType(atomName);
    atom->computeVal(atomName);
    quote->cells.push_back(atom);
    is >> ch;
  }
  
  if (ch == '(') {
    cell = parseSexp(is, env);
  } else {
    is.putback(ch);
    cell = parseAtom(is, env);
  }
  
  if (quote) {
    quote->cells.push_back(cell);
    cell = quote;
  }
    

  return cell;
}

std::vector<std::shared_ptr<Cell> > parse(std::istream& is, Cell::CellEnv& env) {
  std::stringstream ss;
  char ch;
  //format stream to ease parsing
  while(is >> std::noskipws >> ch) {
    if(ch == '(' || ch == '\'') {
      ss << ch << " " ;
    } else if(ch == ')') {
      ss << " " << ch;
    }
    else
      ss << ch;
    }
      
      std::vector<std::shared_ptr<Cell> > cells;
      while(ss.good()) {
      std::shared_ptr<Cell> cell = parseCell(ss, env);
      cells.push_back(cell);
    }
  return cells;
}

std::shared_ptr<Cell> parse2(std::istream& ss, Cell::CellEnv& env)
{
     bool newToken = false;

     std::vector<std::shared_ptr<Sexp> > sexps;
     std::shared_ptr<Sexp> sexp;
     
     int count = 0;
     char ch;
     std::string buffer;
     bool stringing = false;
     for(int cc = 0 ;  ; ++cc)
     {
          ss >> std::noskipws >> ch;
          if(!stringing && (ch == '(' || ch == ' ' || ch == ')' || ch == '\n'))
          {
               if(ch == '(')
               {
		 count++;
                 std::shared_ptr<Sexp> sx = Sexp::New();
                 if(sexps.size())
                   sexp->cells.push_back(sx);
                 sexp = sx;
                 sexps.push_back(sx);
	       }

               newToken = true;
	  
               if(buffer.size())
               {
                 Atom::Type type = Atom::computeType(buffer);
                 std::shared_ptr<Atom> at;
                 if(type == Atom::Symbol)
		   at = SymbolAtom::New(env, buffer);
		 if(type == Atom::String)
                   at = StringAtom::New();
                 if(type == Atom::Real)
                   at = RealAtom::New();
                 
		 at->computeVal(buffer);
		 if(!sexps.size())
		   return at;
   
		 sexp->cells.push_back(at);
		 
		 buffer.resize(0);
               }

               if(ch == ')')
               {
		 count--;
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
      
          if(isalnum(ch) || isoperator(ch) || isquote(ch) || ch == '.'
             || (ch == ' ' && stringing))
            {
              if(ch == '\'' || ch == '`' || ch == ',')
		{
		  std::shared_ptr<Sexp> sx = Sexp::New();
		  if(sexps.size())
		    {
		      sexp->cells.push_back(sx);
		    }
		  sexp = sx;
		  sexps.push_back(sx);

		  std::string atomName = ch == '\'' ? "quote" : (ch == '`' ? "backquote" : "comma"); 
		  std::shared_ptr<Atom> quote = SymbolAtom::New(env, atomName);
		  quote->computeType(atomName);
		  quote->computeVal(atomName);
		     
		  sexp->cells.push_back(quote);
		  if(ch == '\'' || ch == '`')
		    count++;
		}
              else if(ch == '"') {
                buffer.push_back(ch);
                stringing = !stringing;
              }
              else if(ch == ' ' && stringing)
                buffer.push_back(ch);
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

bool evalHelper(std::istream& ss, Cell::CellEnv& env, bool verbose = true)
{
  
  std::vector<std::shared_ptr<Cell> > sexps = parse(ss, env);
  for(auto cIt = sexps.begin() ; cIt != sexps.end() ; cIt++) {
      std::shared_ptr<Cell> sexp = *cIt;
      if(sexp)
	{
      if(sexp->checkSyntax(env))
        {
      std::shared_ptr<Sexp> fun = std::dynamic_pointer_cast<Sexp>(sexp);
      if(fun)
	fun->inferFunctionType(env);
      std::shared_ptr<Cell> res = sexp->eval(env);
      if(verbose) {
      //std::cout << "> " << *sexp << std::endl;
      std::cout << "-> " << *res << std::endl;
    }
    }
      else
        std::cout << "Syntax Error" << std::endl;
      //return true;
    }
  else
    return false;
    }
  return true;
}

void loadFile(const std::string& file, Cell::CellEnv& env, bool verbose = true)
{
  std::ifstream in(file);
  while(!in.eof())
    if(!evalHelper(in, env, verbose))
      break;
}

int main(int argc, char* argv[])
{ 
  Cell::CellEnv env; 
  std::string in;
  std::string curLine;
  
  
  std::stringstream special("( load \"./special.so\" \"registerSpecialHandlers\" )");
  evalHelper(special, env, false);
 
  std::stringstream core("( load \"./core.so\" \"registerCoreHandlers\" )");
  evalHelper(core, env, false);
  
  std::stringstream func("( load \"./functional.so\" \"registerFunctionalHandlers\" )");
  evalHelper(func, env, false);
  std::stringstream str("( load \"./string.so\" \"registerStringHandlers\" )");
  evalHelper(str, env, false);
  std::stringstream list("( load \"./list.so\" \"registerListHandlers\" )");
  evalHelper(list, env, false);
  std::stringstream bench("( load \"./bench.so\" \"registerBenchHandlers\" )");
  evalHelper(bench, env, false);
  std::stringstream comp("( load \"./compiler.so\" \"registerCompilerHandlers\" )");
  evalHelper(comp, env, false);
  
  bool verbose = true;
  if(argc >= 3) 
    if(std::string("-q").compare(argv[2]) == 0)
      verbose = false;

  loadFile("lib/core.cl", env, verbose);    

  if(argc >= 2)
    loadFile(argv[1], env, verbose);
 

  while(true)
    {
      std::cout << (curLine.size() ? "     > " : "elisp> ");
      std::getline(std::cin, in);
      if(std::cin.eof()) {
	std::cout << "goobye" << std::endl;
	break;
      }
      size_t posc = in.find(";");
      size_t posp = in.find("(");
      if(posc < posp)
        {
          std::cout << "skip comment" << std::endl;
					continue;
        }

      curLine += " " +  in;
      
      std::stringstream cl(curLine);

      if(evalHelper(cl, env, true))
        curLine = "";
    }
  
  return 0;
}
