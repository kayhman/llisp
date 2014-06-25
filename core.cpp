#include <ostream>
#include "cell.h"


extern "C" void registerCoreHandlers(Cell::CellEnv& env)
{
  std::shared_ptr<Atom> print = SymbolAtom::New(env, "print");
  print->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::for_each(sexp->cells.begin()+1, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){std::cout << *cell->eval(env) << " ";});
    std::cout << std::endl;
    return sexp->cells.back()->eval(env);
  };

  std::shared_ptr<Atom> plus = SymbolAtom::New(env, "+");
  std::dynamic_pointer_cast<SymbolAtom>(plus)->prototype = Prototype("ff*");
  plus->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> res = RealAtom::New();
    res->real = 0;
    std::for_each(sexp->cells.begin()+1, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){res->real += cell->eval(env)->real;});

    return res;
  };

  std::shared_ptr<Atom> minus = SymbolAtom::New(env, "-");
  std::dynamic_pointer_cast<SymbolAtom>(minus)->prototype = Prototype("ff*");
  minus->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> res = RealAtom::New();
    res->real = sexp->cells[1]->eval(env)->real;
    std::for_each(sexp->cells.begin()+2, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){res->real -= cell->eval(env)->real;});

    return res;
  };

  std::shared_ptr<Atom> mult = SymbolAtom::New(env, "*");
  std::dynamic_pointer_cast<SymbolAtom>(mult)->prototype = Prototype("ff*");
  mult->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> res = RealAtom::New();
    res->real = 1.0;
    std::for_each(sexp->cells.begin()+1, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){res->real *= cell->eval(env)->real;});

    return res;
  };

  std::shared_ptr<Atom> div = SymbolAtom::New(env, "/");
  std::dynamic_pointer_cast<SymbolAtom>(div)->prototype = Prototype("ff*");
  div->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> res = RealAtom::New();
    res->real = sexp->cells[1]->eval(env)->real;
    std::for_each(sexp->cells.begin()+2, sexp->cells.end(), [&](std::shared_ptr<Cell> cell){res->real /= cell->eval(env)->real;});
    return res;
  };

  std::shared_ptr<Atom> inf = SymbolAtom::New(env, "<");
  std::dynamic_pointer_cast<SymbolAtom>(inf)->prototype = Prototype("fff");
  inf->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> m1 = sexp->cells[1];
    std::shared_ptr<Cell> m2 = sexp->cells[2];
 
    double a1 = m1->eval(env)->real;
    double a2 = m2->eval(env)->real;
  
    std::shared_ptr<Cell> res = sexp->evaluated.lock();//RealAtom::New();
    if (a1 < a2)
      res->real = 1.0;
    else
      res->real = 0.;
    return res;
  };

  std::shared_ptr<Atom> sup = SymbolAtom::New(env, ">");
  std::dynamic_pointer_cast<SymbolAtom>(sup)->prototype = Prototype("fff");
  sup->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> m1 = sexp->cells[1];
    std::shared_ptr<Cell> m2 = sexp->cells[2];
    
    double a1 = m1->eval(env)->real;
    double a2 = m2->eval(env)->real;
 
    std::shared_ptr<Cell> res = sexp->evaluated.lock();//RealAtom::New();
    if (a1 > a2)
      res->real = 1.0;
    else
      res->real = 0.;
    return res;
  };
}
