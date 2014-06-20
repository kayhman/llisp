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
bool isquote(char c);
struct Sexp;

struct Cell
{
  enum Quoting {Quote, BackQuote, NoneQ};
  enum Type {Symbol, Real, String, Unknown};
  bool compiled;
  typedef Env<std::string, std::shared_ptr<Cell> > CellEnv;
  virtual ~Cell() {};
Cell() :quoting(NoneQ), compiled(false) {};
  mutable std::function<std::shared_ptr<Cell> (Sexp*, Cell::CellEnv&)> closure;
  mutable std::string val;
  mutable double real;

  Quoting quoting;
  std::weak_ptr<Cell> evaluated;
  virtual std::shared_ptr<Cell> eval(CellEnv& env) = 0;
  virtual Type computeType() const = 0;
  bool checkSyntax() const;

  friend std::ostream& operator<< (std::ostream& stream, const Cell& cell);
  friend std::ostream& operator>> (std::istream& stream, Cell& cell);
};

struct Sexp : public Cell
{
  std::vector<std::shared_ptr<Cell> > cells;
  virtual ~Sexp() {};
  virtual std::shared_ptr<Cell> eval(CellEnv& env);
  virtual Type computeType() const;
  friend std::ostream& operator<< (std::ostream& stream, const Sexp& cell);
  static std::shared_ptr<Sexp> New();
  static std::list<std::shared_ptr<Sexp> > gc;
  static std::list<std::shared_ptr<Sexp> > pool;
  static void initGC();
};


struct Atom : public Cell
{
  virtual ~Atom() {};  
  static Type computeType(const std::string& code);
  virtual Type computeType() const = 0;
  virtual void computeVal(const std::string& code) const = 0;
  virtual std::shared_ptr<Cell> eval(CellEnv& env) = 0;
  Atom(const std::string& val) {this->val = val;};
 
  friend std::ostream& operator<< (std::ostream& stream, const Atom& cell);
 protected:
 Atom() {};
};


struct RealAtom : public Atom
{
  virtual std::shared_ptr<Cell> eval(CellEnv& env);
  virtual Type computeType() const {return Type::Real;};
  void computeVal(const std::string& code) const;
  friend std::ostream& operator<< (std::ostream& stream, const RealAtom& cell);
  static std::shared_ptr<RealAtom> New();
  static std::list<std::shared_ptr<RealAtom> > gc;
  static std::list<std::shared_ptr<RealAtom> > pool;
  static void initGC();
 protected:
  RealAtom() {};
};


struct StringAtom : public Atom
{
  virtual std::shared_ptr<Cell> eval(CellEnv& env);
  virtual Type computeType() const {return Type::String;};
  void computeVal(const std::string& code) const;
  friend std::ostream& operator<< (std::ostream& stream, const StringAtom& cell);
  static std::shared_ptr<StringAtom> New();
  static std::list<std::shared_ptr<StringAtom> > gc;
  static std::list<std::shared_ptr<StringAtom> > pool;
  static void initGC();
 protected:
  StringAtom() {};
};

struct SymbolAtom : public Atom
{
  std::shared_ptr<Cell> code;
  std::shared_ptr<Sexp> args;
  virtual std::shared_ptr<Cell> eval(CellEnv& env);
  Type retType;
  virtual Type computeType() const;
  void computeVal(const std::string& code) const;
  friend std::ostream& operator<< (std::ostream& stream, const SymbolAtom& cell);
  static std::shared_ptr<SymbolAtom> New(); 
  static std::shared_ptr<Atom> New(Cell::CellEnv& env, const std::string& name); 
  static std::list<std::shared_ptr<SymbolAtom> > gc;
  static std::list<std::shared_ptr<SymbolAtom> > pool;
  static void initGC();
 protected:
  SymbolAtom() {};
};
