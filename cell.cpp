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
  stream << atom.val;
  return stream;
}

void Atom::computeType(const std::string& code)
{
     this->type = Atom::Symbol;
     if(isdigit(code.front()) ||  isoperator(code.front()))
          this->type = Atom::Real;
     if(code.front() == '"' &&  code.back() == '"')
          this->type = Atom::String;
}

void Atom::computeVal(const std::string& code) const
{
  this->val = code;
}


std::shared_ptr<Cell> Atom::eval(CellEnv& env)
{
  if(this->type == Atom::Symbol &&  env.find(this->val))
      return env[this->val]->eval(env);
  else if(this->type == Atom::String)
    {
      std::shared_ptr<Cell> res(new Atom);
      *res = *this;
      res->val = this->val.substr(1, this->val.size()-2); 
      return res;
    }
  else
    {
      std::shared_ptr<Cell> res(new Atom);
      *res = *this;
      return res;
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
  if(cl->val.compare("load") == 0)   
    {
      loadHandlers(this->cells[1]->eval(env)->val,
		   this->cells[2]->eval(env)->val,
		   env);
      std::shared_ptr<Cell> res(new Atom);
      res->val =  "loaded";
      return res;
    }

  if(env.evalHandlers.find(cl->val) != env.evalHandlers.end())
      return env.evalHandlers[cl->val](this, env);

  if(cl->val.compare("printenv") == 0)   
    {
      
    }


  if(env.find(cl->val))
    return  env[cl->val]->closure(this, std::vector<std::shared_ptr<Cell> >(this->cells.begin()+1, this->cells.end()));

  return std::shared_ptr<Cell>(new Atom);  
}
