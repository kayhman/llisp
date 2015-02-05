#include "cell.h"
#include <dlfcn.h>

typedef void* (*arbitrary)();

void* externalCall(const std::string& lib, const std::string& fun)
{
  void* handle = dlopen(lib.c_str(), RTLD_LAZY);
  arbitrary handler;

  if (!handle) {
    std::cout << "Cannot open library: " << dlerror() << '\n';
    return NULL;
  }
  
  // load the symbol
  //typedef void (*hello_t)(Cell::CellEnv& env);
  
  // reset errors
  dlerror();
  *(void**)(&handler)  = dlsym(handle, fun.c_str());
  const char *dlsym_error = dlerror();
  if (dlsym_error) {
    std::cout << "Cannot load symbol 'hello': " << dlsym_error <<
      '\n';
    dlclose(handle);
    return NULL;
  }
  
  // use it to do the calculation
  //handler(env);
  
  // close the library
  dlclose(handle);
  return NULL;
}


extern "C" void registerNativeHandlers(Cell::CellEnv& env)
{

  //(native "lib.so" "func" args)
  std::shared_ptr<Atom> native = SymbolAtom::New(env, "native");
  native->closure = [](Sexp* sexp, Cell::CellEnv& env) {
    std::shared_ptr<Cell> lib = sexp->cells[1];
    std::shared_ptr<Cell> fun = sexp->cells[2];
    
    std::string libName = lib->eval(env)->val;
    std::string funName = fun->eval(env)->val;

    

    return RealAtom::New();
  };
}

extern "C" void testo(void)
{
  std::cout << "coucou" << std::endl;
}

extern "C" void testadd(double a, double b)
{
  std::cout << "coucou " << a + b << std::endl;
}

extern "C" void testmult(double a, double b)
{
  std::cout << "coucou " << a * b -1 << std::endl;
}


extern "C" void testconcat(const char* a, const char* b)
{
  std::cout << "coucou " << a << " and " <<  b << std::endl;
}


extern "C" void testconcat2(const std::string& a, const std::string& b)
{
  std::cout << "coucou " << a << " and " <<  b << std::endl;
}

