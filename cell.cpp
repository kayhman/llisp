#include "cell.h"

template<typename K, typename V> std::map<K, V> Env<K,V>::top;
template<typename K, typename V> std::map<K, std::function<V(Sexp* sexp, Env<K,V>& env)> > Env<K,V>::evalHandlers;


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
      std::shared_ptr<Cell> res(new Atom());
      *res = *this;
      res->val = this->val.substr(1, this->val.size()-2); 
      return res;
    }
  else
    {
      std::shared_ptr<Cell> res(new Atom());
      *res = *this;
      return res;
    }
}

std::shared_ptr<Cell> Sexp::eval(CellEnv& env)
{
  std::shared_ptr<Cell> cl = this->cells[0];

  if(env.evalHandlers.find(cl->val) != env.evalHandlers.end())
    return env.evalHandlers[cl->val](this, env);

  if(cl->val.compare("printenv") == 0)   
    {
      
    }

  if(env.find(cl->val))
    return  env[cl->val]->closure(this, std::vector<std::shared_ptr<Cell> >(this->cells.begin()+1, this->cells.end()));

  return std::shared_ptr<Cell>(new Atom());  
}

template class  Env<std::string,std::shared_ptr<Cell> >;
