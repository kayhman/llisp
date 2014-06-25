#include "cell.h"
#include <dlfcn.h>



Prototype::Prototype(const std::string& protoString) :
  protoString(protoString)
{
  
}

Prototype::Prototype() :
  protoString("fff") //TODO :remove this dirty hack once the type inference works.
{
  
}


const Cell::Type Prototype::convert(const char c)
{
  switch(c) {
  case 'f':
    return Cell::Type::Real;
  case 's':
    return Cell::Type::String;
  case 'a':
    return Cell::Type::Symbol;
  case 'l':
    return Cell::Type::List;
  default:
    return Cell::Type::Unknown;
  }
}

const char Prototype::convert(const Cell::Type t)
{
  switch(t) {
  case Cell::Type::Real:
    return 'f';
  case Cell::Type::String:
    return 's';
  case Cell::Type::Symbol:
    return 'a';
  case Cell::Type::List:
    return 'l';
  case Cell::Type::Unknown:
    return 'u';
  }
}

const Cell::Type Prototype::returnType() const
{
  return Prototype::convert(protoString[0]);
}

const Cell::Type Prototype::argType(const int& i) const
{
  if(protoString.size() == 3 && protoString[2] == '*') {
    return Prototype::convert(protoString[1]);
  }
  else {
    if(i < protoString.size()) {
      return Prototype::convert(protoString[i]);
    }
    else
      return Cell::Type::Unknown;
  }
}


bool isoperator(char c) {
     std::string const valid_chars = "+*-/!=<>\"";
     return valid_chars.find(c) != std::string::npos;
}
bool isquote(char c) {
     std::string const valid_chars = "'`,";
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

bool Cell::checkSyntax(CellEnv& env) const
{
  return true;//TODO : check syntax ! this->computeType(env) != Cell::Type::Unknown;
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
  auto it = env.find(this->val);
  if(it != env.end())// && it->second.get() != this)
    return it->second->eval(env);
  else
    {
      return evaluated.lock();
    }
}

Cell::Type SymbolAtom::evalType(CellEnv& env)
{
  return Prototype::convert(this->prototype.protoString[0]);
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

bool incompatibleType(Cell::Type ta, Cell::Type tb)
{
  return (ta == Cell::Type::Real && tb == Cell::Type::String) ||
    (tb == Cell::Type::Real && ta == Cell::Type::String);
}


std::ostream& operator<< (std::ostream& stream, const Prototype& proto)
{
  stream << proto.protoString;
  return stream;
}

Cell::Type Sexp::inferType(const std::string& symbolName) const
{
  if(this->cells.size()) {
    std::shared_ptr<SymbolAtom> fun = std::dynamic_pointer_cast<SymbolAtom>(this->cells[0]);
    if(fun) {
      const Prototype proto = fun->prototype;
      int argIdx = 1;
      for(auto cIt = this->cells.begin() + 1 ; cIt != this->cells.end() ; cIt++) {
        std::shared_ptr<SymbolAtom> arg = std::dynamic_pointer_cast<SymbolAtom>(*cIt);
        if(arg) {
          if(arg->val.compare(symbolName) == 0) {
            return proto.argType(argIdx);
          }
        }
        else {
          std::shared_ptr<Sexp> sx = std::dynamic_pointer_cast<Sexp>(*cIt);
          if(sx) {
            Cell::Type type = sx->inferType(symbolName);
            if(type != Cell::Type::Unknown)
              return type;
          }
        }
        argIdx++;
      }
    }
  }
  
  return Cell::Type::Unknown;
}

Cell::Type Sexp::inferFunctionType(Cell::CellEnv& env) const
{
  if(this->cells[0]->val.compare("defun") == 0) {
    std::shared_ptr<Atom> fname = SymbolAtom::New(env, this->cells[1]->val);
    std::shared_ptr<Sexp> args = std::dynamic_pointer_cast<Sexp>(this->cells[2]);
    std::shared_ptr<Sexp> body = std::dynamic_pointer_cast<Sexp>(this->cells[3]);

    std::stringstream ss;

    for(auto aIt = args->cells.begin() ; aIt != args->cells.end() ; aIt++) {
      Cell::Type t = body->inferType((*aIt)->val);
      ss << Prototype::convert(t);

      std::shared_ptr<SymbolAtom> sArg = std::dynamic_pointer_cast<SymbolAtom>(*aIt);
      if(sArg)
	sArg->prototype.protoString = Prototype::convert(t);
      
    }
    const std::string argsTypes = ss.str();
    ss.str("");
    ss << Prototype::convert(body->evalType(env)) << argsTypes;

    std::cout << "function type : " << ss.str() << std::endl;
  }
}

std::shared_ptr<Cell> Sexp::eval(CellEnv& env)
{
  std::shared_ptr<Cell> cl = this->cells[0];
  if(cl->closure)
    {
      return cl->closure(this, env);
    }

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
      for(auto cIt = env.func.begin() ; cIt != env.func.end() ; cIt++)
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

Cell::Type Sexp::evalType(CellEnv& env)
{
  std::shared_ptr<Cell> cl = this->cells[0];
  if(cl->closure) {
    return cl->closureType(this, env);
  }
  return Cell::Type::Unknown;
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

