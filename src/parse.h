#pragma once
#include <memory>
//#include "cell.h"
#include <vector>

struct Cell
{
  enum Type {Symbol, Real, String, List, Unknown};
  virtual ~Cell() {};
  Cell()  {};
  mutable std::string val;
  
  friend std::ostream& operator<< (std::ostream& stream, const Cell& cell);
};

struct Sexp : public Cell
{
  std::vector<std::shared_ptr<Cell> > cells;
  virtual ~Sexp() {};

  friend std::ostream& operator<< (std::ostream& stream, const Sexp& cell);
  static std::shared_ptr<Sexp> New();
};


struct Atom : public Cell
{
  virtual ~Atom() {};  
  Type type;
  static Type computeType(const std::string& code);
 Atom(Type type, const std::string& val) : type(type) {this->val = val;};
 
  friend std::ostream& operator<< (std::ostream& stream, const Atom& cell);
  static std::shared_ptr<Atom> New(Type type, const std::string& name);
};

std::vector<std::shared_ptr<Cell> > parse(std::istream& is);
bool evalHelper(std::istream& ss, bool verbose = true);
void loadFile(const std::string& file, bool verbose = true);
std::shared_ptr<Cell> parseCell(std::istream& is);
