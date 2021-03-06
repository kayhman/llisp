#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <fstream>
#include <memory>
#include "environment.h"

bool isoperator(char c);
bool isquote(char c);
struct Sexp;

struct Cell
{
  enum Type {Symbol, Real, String, List, Unknown};
  bool compiled;
  typedef Env<std::string, std::shared_ptr<Cell> > CellEnv;
  virtual ~Cell() {};
  Cell() : compiled(false), real(NAN) {};
  mutable std::function<std::shared_ptr<Cell> (Sexp*, Cell::CellEnv&)> closure;
  mutable std::function<Type (Sexp*, Cell::CellEnv&)> closureType;
  mutable std::string val;
  mutable double real;

  std::weak_ptr<Cell> weakRef;
  virtual std::shared_ptr<Cell> eval(CellEnv& env) = 0;
  virtual Type evalType(CellEnv& env) = 0;
  virtual std::shared_ptr<Cell> duplicate() = 0;


  static std::shared_ptr<Cell> nil;
  static std::shared_ptr<Cell> t;
  static std::vector<std::function<void (Cell*, CellEnv&)> > hooks;

  friend std::ostream& operator<< (std::ostream& stream, const Cell& cell);
  friend std::ostream& operator>> (std::istream& stream, Cell& cell);
};

class Prototype
{
  //f -> Cell::Type::Real:
  //s -> Cell::Type::String:
  //a -> Cell::Type::Symbol:
  //l -> Cell::Type::List:
  //u -> Cell::Type::Unknown:
 public:
  std::string protoString;
  Prototype(const std::string& protoString);
  Prototype();
  static const Cell::Type convert(const char c);
  static const char convert(const Cell::Type t);
  const Cell::Type returnType() const;
  const Cell::Type argType(const int& i) const;
  friend std::ostream& operator<< (std::ostream& stream, const Prototype& proto);
};

struct Sexp : public Cell
{
  std::vector<std::shared_ptr<Cell> > cells;
  virtual ~Sexp() {};
  virtual std::shared_ptr<Cell> eval(CellEnv& env);
  virtual Type evalType(CellEnv& env);
  Type inferType(const std::string& symbolName) const;
  Type inferReturnType(CellEnv& env) const;
  Type inferFunctionType(CellEnv& env) const;
  virtual std::shared_ptr<Cell> duplicate();

  friend std::ostream& operator<< (std::ostream& stream, const Sexp& cell);
  static std::shared_ptr<Sexp> New();
 protected:
  Sexp() {};
};


struct Atom : public Cell
{
  virtual ~Atom() {};  
  static Type computeType(const std::string& code);
  virtual void computeVal(const std::string& code) const = 0;
  virtual std::shared_ptr<Cell> eval(CellEnv& env) = 0;
  virtual Type evalType(CellEnv& env) = 0;
  
  friend std::ostream& operator<< (std::ostream& stream, const Atom& cell);
 protected:
 Atom() {};
 Atom(const std::string& val) {this->val = val;};
 
};


struct RealAtom : public Atom
{
  virtual std::shared_ptr<Cell> eval(CellEnv& env);
  virtual Type evalType(CellEnv& env) {return Type::Real;};
  void computeVal(const std::string& code) const;
  friend std::ostream& operator<< (std::ostream& stream, const RealAtom& cell);
  static std::shared_ptr<RealAtom> New();
  virtual std::shared_ptr<Cell> duplicate();
  virtual ~RealAtom() {};  
 
 protected:
  RealAtom() {};
};


struct StringAtom : public Atom
{
  virtual std::shared_ptr<Cell> eval(CellEnv& env);
  virtual Type evalType(CellEnv& env) {return Type::String;};
  void computeVal(const std::string& code) const;
  friend std::ostream& operator<< (std::ostream& stream, const StringAtom& cell);
  static std::shared_ptr<StringAtom> New();
  virtual std::shared_ptr<Cell> duplicate();
  virtual ~StringAtom() {}

 protected:
  StringAtom() {};
};

struct SymbolAtom : public Atom
{
  std::shared_ptr<Cell> code;
  std::shared_ptr<Sexp> args;
  Prototype prototype;
  virtual std::shared_ptr<Cell> eval(CellEnv& env);
  virtual Type evalType(CellEnv& env);
  void computeVal(const std::string& code) const;
  friend std::ostream& operator<< (std::ostream& stream, const SymbolAtom& cell);
  static std::shared_ptr<SymbolAtom> New(); 
  static std::shared_ptr<Atom> New(Cell::CellEnv& env, const std::string& name); 
  virtual std::shared_ptr<Cell> duplicate();
  virtual ~SymbolAtom() {};  
  
 protected:
 SymbolAtom() {
   this->closureType = [this](Sexp* sexp, Cell::CellEnv& env) { return this->prototype.returnType(); };
 };
};
