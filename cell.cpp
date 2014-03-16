#include "cell.h"
#include <dlfcn.h>

bool isoperator(char c) {
     std::string const valid_chars = "+*-/!=<>\"'`,";
     return valid_chars.find(c) != std::string::npos;
}

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
  const RealAtom * real = dynamic_cast<const RealAtom*>(&atom);
  const StringAtom * string = dynamic_cast<const StringAtom*>(&atom);
  const SymbolAtom * symbol = dynamic_cast<const SymbolAtom*>(&atom);

  if(real)
    stream << *real;
  if(string)
    stream << *string;
  if(symbol)
    stream << *symbol;
  return stream;
}

std::ostream& operator<< (std::ostream& stream, const RealAtom& atom)
{
  stream << atom.real;
  return stream;
}

std::ostream& operator<< (std::ostream& stream, const StringAtom& atom)
{
  stream << atom.val;
  return stream;
}

std::ostream& operator<< (std::ostream& stream, const SymbolAtom& atom)
{
  stream << atom.val;
  return stream;
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

void RealAtom::computeVal(const std::string& code) const
{
  this->real = atof(code.c_str());
  this->val = code;
}

void StringAtom::computeVal(const std::string& code) const
{
  this->val = code.substr(1, code.size()-2);
}

void SymbolAtom::computeVal(const std::string& code) const
{
  this->val = code;
}


std::shared_ptr<Cell> RealAtom::eval(CellEnv& env)
{
  return evaluated.lock();
}

std::shared_ptr<Cell> StringAtom::eval(CellEnv& env)
{
  return evaluated.lock();
}

std::shared_ptr<Cell> SymbolAtom::eval(CellEnv& env)
{
  std::cout << "eval " << this->val << std::endl;
  auto it = env.find(this->val);
  //std::cout << "find  " << *it->second << std::endl;
  if(it != env.end())// && it->second.get() != this)
    return it->second->eval(env);
  else
    {
      return evaluated.lock();
    }
}

std::shared_ptr<Sexp> Sexp::New()
{
  std::shared_ptr<Sexp> sexp(new Sexp);// = pool.back();
  //pool.pop_back();
  sexp->evaluated = sexp;
  //gc.push_back(sexp);
  return sexp;
}

std::shared_ptr<RealAtom> RealAtom::New()
{
  std::shared_ptr<RealAtom> atom(new RealAtom);// = pool.back();
  //  pool.pop_back();
  atom->evaluated = atom;
  //  gc.push_back(atom);
  return atom;
}

std::shared_ptr<StringAtom> StringAtom::New()
{
  std::shared_ptr<StringAtom> atom(new StringAtom);// = pool.back();
  //pool.pop_back();
  atom->evaluated = atom;
  //gc.push_back(atom);
  return atom;
}

std::shared_ptr<SymbolAtom> SymbolAtom::New()
{
  std::shared_ptr<SymbolAtom> atom(new SymbolAtom);// = pool.back();
  //  pool.pop_back();
  atom->evaluated = atom;
  //  gc.push_back(atom);
  return atom;
}

std::shared_ptr<Atom> SymbolAtom::New(Cell::CellEnv& env, const std::string& name)
{
  auto clIt = env.func.find(name);
  if(clIt != env.func.end())
    {
      std::cout << "found " << name << " " << std::dynamic_pointer_cast<Atom>(clIt->second) << std::endl;
      return std::dynamic_pointer_cast<Atom>(clIt->second);
    }
  else
    {
      std::shared_ptr<SymbolAtom> atom(new SymbolAtom);// = pool.back();
      env.func[name] = atom;
      //  pool.pop_back();
      atom->evaluated = atom;
      //  gc.push_back(atom);
      return atom;
    }
}

int loadHandlers(const std::string& lib, const std::string& handlerName, Cell::CellEnv& env)
{
  void* handle = dlopen(lib.c_str(), RTLD_LAZY);
    
    if (!handle) {
      std::cout << "Cannot open library: " << dlerror() << '\n';
        return 1;
    }
    
    // load the symbol
    typedef void (*hello_t)(Cell::CellEnv& env);

    // reset errors
    dlerror();
    hello_t handler = (hello_t) dlsym(handle, handlerName.c_str());
    const char *dlsym_error = dlerror();
    if (dlsym_error) {
      std::cout << "Cannot load symbol 'hello': " << dlsym_error <<
            '\n';
        dlclose(handle);
        return 1;
    }
    
    // use it to do the calculation
    handler(env);
    
    // close the library
    //dlclose(handle);
    return 0;
}

std::shared_ptr<Cell> Sexp::eval(CellEnv& env)
{
  std::shared_ptr<Cell> cl = this->cells[0];
  if(cl->closure)
    return cl->closure(this, env);

  if(cl->val.compare("load") == 0)   
    {
      loadHandlers(this->cells[1]->eval(env)->val,
		   this->cells[2]->eval(env)->val,
		   env);
      std::shared_ptr<Cell> res(StringAtom::New());
      res->val =  "loaded";
      return res;
    }

  if(cl->val.compare("printenv") == 0)   
    {
      std::shared_ptr<Cell> res = StringAtom::New();
      res->val = "printenv";
      for(auto envIt = env.envs.rbegin() ; envIt != env.envs.rend() ; envIt++)
	for(auto cIt = (*envIt)->begin() ; cIt != (*envIt)->end() ; cIt++)
	  {
	    std::shared_ptr<Cell> cell = (cIt->second); 
	    std::cout << cIt->first << " -> " << *cell << std::endl;
	  }
      return res;
  }

  auto evalIt = env.evalHandlers.find(cl->val);
  if(evalIt != env.evalHandlers.end())
    return (evalIt->second)(this, env);
  

  return std::shared_ptr<Cell>(StringAtom::New());    
}

void Sexp::initGC()
{
  for(int i = 0 ; i < 1024*100 ; i++)
    pool.push_back(std::shared_ptr<Sexp>(new Sexp()));
}

void RealAtom::initGC()
{
  for(int i = 0 ; i < 1024*100 ; i++)
    pool.push_back(std::shared_ptr<RealAtom>(new RealAtom()));
}

void StringAtom::initGC()
{
  for(int i = 0 ; i < 1024*100 ; i++)
    pool.push_back(std::shared_ptr<StringAtom>(new StringAtom()));
}

void SymbolAtom::initGC()
{
  for(int i = 0 ; i < 1024*10 ; i++)
    pool.push_back(std::shared_ptr<SymbolAtom>(new SymbolAtom()));
}

std::list<std::shared_ptr<Sexp> > Sexp::gc;
std::list<std::shared_ptr<RealAtom> > RealAtom::gc;
std::list<std::shared_ptr<StringAtom> > StringAtom::gc;
std::list<std::shared_ptr<SymbolAtom> > SymbolAtom::gc;

std::list<std::shared_ptr<Sexp> > Sexp::pool;
std::list<std::shared_ptr<RealAtom> > RealAtom::pool;
std::list<std::shared_ptr<StringAtom> > StringAtom::pool;
std::list<std::shared_ptr<SymbolAtom> > SymbolAtom::pool;

