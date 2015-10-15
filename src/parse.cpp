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
