#include "parse.h"
#include <iostream>
#include <istream>
#include <fstream>
#include <sstream>
#include <algorithm>

std::ostream& operator<< (std::ostream& stream, const Cell& cell)
{
  const Atom * atom = dynamic_cast<const Atom*>(&cell);
  const Sexp * sexp = dynamic_cast<const Sexp*>(&cell);

  if(atom)
       stream << *atom;
 
  if(sexp)
       stream << *sexp;

  return stream;
}

std::ostream& operator<< (std::ostream& stream, const Sexp& cell)
{
  stream << "(";
  std::for_each(cell.cells.begin(), cell.cells.end(), [&](std::shared_ptr<Cell> cell){stream << *cell << " ";}); 
  stream << ")";

  return stream;
}

std::ostream& operator<< (std::ostream& stream, const Atom& atom)
{
  stream << atom.val;
  return stream;
}


std::shared_ptr<Sexp> Sexp::New()
{
  std::shared_ptr<Sexp> sexp(new Sexp);
  return sexp;
}

std::shared_ptr<Atom> Atom::New(Cell::Type type, const std::string& name)
{
  std::shared_ptr<Atom> atom(new Atom(type, name));
  return atom;
}

bool isoperator(char c) {
     std::string const valid_chars = "+*-/!=<>\"";
     return valid_chars.find(c) != std::string::npos;
}

Atom::Type Atom::computeType(const std::string& code)
{
  if(code.front() == '"' &&  code.back() == '"')
    return Atom::String;
  if(isdigit(code.front()) ||  
     (isoperator(code.front()) && code.size() > 1))
    return  Atom::Real;
  if(isdigit(code.front()) && code.size() == 1)
    return  Atom::Real;
  
  return Atom::Symbol;
}

////////////////////////
//   Parse code       //
////////////////////////

std::shared_ptr<Cell> parseString(std::istream& is) {
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

  std::shared_ptr<Cell> at = Atom::New(Cell::Type::String, atom);
  return at;
}

std::shared_ptr<Cell> parseAtom(std::istream& is) {
  std::shared_ptr<Cell> at;  
  char ch;
  is >> ch;
  is.putback(ch);
  if (ch == '"') {
    at = parseString(is);
  } else {
    std::string atom;
    is >> atom;
    Atom::Type type = Atom::computeType(atom);
    at = Atom::New(type, atom);
  }

  return at; 
}

std::shared_ptr<Cell> parseSexp(std::istream& is) {
  char ch;
  std::shared_ptr<Sexp> sx = Sexp::New();
  while(is >> ch) {
    if (ch == ')') {
      return sx;
    } else {
      is.putback(ch);
      std::shared_ptr<Cell> cell = parseCell(is);
      sx->cells.push_back(cell);
    }
  }
}


std::shared_ptr<Cell> parseCell(std::istream& is) {
  char ch;
  is >> ch;

  std::shared_ptr<Sexp> quote;
  std::shared_ptr<Cell> cell;

  if (ch == '\'' || ch == '`' || ch == ',') {
    quote = Sexp::New();
    std::string atomName = ch == '\'' ? "quote" : (ch == '`' ? "backquote" : "comma"); 
    std::shared_ptr<Atom> atom = Atom::New(Cell::Type::Symbol, atomName);

    quote->cells.push_back(atom);
    is >> ch;
  }
  
  if (ch == '(') {
    cell = parseSexp(is);
  } else {
    is.putback(ch);
    cell = parseAtom(is);
  }
  
  if (quote) {
    quote->cells.push_back(cell);
    cell = quote;
  }
    

  return cell;
}

std::vector<std::shared_ptr<Cell> > parse(std::istream& is) {
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
    std::shared_ptr<Cell> cell = parseCell(ss);
    cells.push_back(cell);
  }
  return cells;
}

bool evalHelper(std::istream& ss, bool verbose)
{
  
  std::vector<std::shared_ptr<Cell> > sexps = parse(ss);
  for(auto cIt = sexps.begin() ; cIt != sexps.end() ; cIt++) {
    std::shared_ptr<Cell> sexp = *cIt;
    if(sexp)
      {
	std::shared_ptr<Sexp> fun = std::dynamic_pointer_cast<Sexp>(sexp);
	if(verbose) {
	  std::cout << "> " << *sexp << std::endl;
	  //std::cout << "-> " << *res << std::endl;
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

void loadFile(const std::string& file, bool verbose)
{
  std::ifstream in(file);
  while(!in.eof())
    if(!evalHelper(in, verbose))
      break;
}
