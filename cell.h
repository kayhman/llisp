#pragma once
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
#include <memory>
#include <regex>
#include "environment.h"

bool isoperator(char c);

struct Cell
{
  enum Quoting {Quote, BackQuote, NoneQ};
  typedef Env<std::string, std::shared_ptr<Cell> > CellEnv;
  virtual ~Cell() {};
  Cell() :quoting(NoneQ) {};
  mutable std::function<std::shared_ptr<Cell>(Cell* self, std::vector<std::shared_ptr<Cell> >)> closure;
  mutable std::string val;
  Quoting quoting;

  virtual std::shared_ptr<Cell> eval(CellEnv& env) = 0;

  friend std::ostream& operator<< (std::ostream& stream, const Cell& cell);
  friend std::ostream& operator>> (std::istream& stream, Cell& cell);
};

struct Sexp : public Cell
{
  std::vector<std::shared_ptr<Cell> > cells;
  virtual ~Sexp() {};
  virtual std::shared_ptr<Cell> eval(CellEnv& env);
  friend std::ostream& operator<< (std::ostream& stream, const Sexp& cell);
};


struct Atom : public Cell
{
  enum Type {Symbol, Real, String, Closure};
  Type type;
  virtual ~Atom() {};  
  void computeType(const std::string& code);
  void computeVal(const std::string& code) const;
  virtual std::shared_ptr<Cell> eval(CellEnv& env);
  Atom(){};
  Atom(const std::string& val) {this->val = val;};
 
  friend std::ostream& operator<< (std::ostream& stream, const Atom& cell);
};
