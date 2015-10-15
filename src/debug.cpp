#include "cell.h"

void trace(Cell* c, Cell::CellEnv& env)
{
  if(env.find("debug") == env.end())
    return;
  if(env["debug"]->real == 0.0)
    return;
  Sexp* sexp = dynamic_cast<Sexp*>(c);
  
  if(sexp) { 
    if(sexp->cells.size()) {
      if(sexp->cells[0]->closure)
        std::cout << "trace -> " << *c << std::endl;
    }
  }
}


extern "C" void registerDebugHandlers(Cell::CellEnv& env)
{
  
  auto debug = RealAtom::New();
  debug->real = 0.0; // debug is disabled by default
  env.top["debug"] = debug;
  
  Cell::hooks.push_back(trace);
}
