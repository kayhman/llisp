#include "parse.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <sstream>

int main(int argc, char* argv[])
{ 
  Cell::CellEnv env; 
  std::string in;
  std::string curLine;
  char* line_read = NULL;

  std::stringstream special("(load \"./special.so\" \"registerSpecialHandlers\")");
  evalHelper(special, env, false);
  std::stringstream core("(load \"./core.so\" \"registerCoreHandlers\")");
  evalHelper(core, env, false);
  std::stringstream func("(load \"./functional.so\" \"registerFunctionalHandlers\")");
  evalHelper(func, env, false);
  std::stringstream str("(load \"./string.so\" \"registerStringHandlers\")");
  evalHelper(str, env, false);
  std::stringstream list("(load \"./list.so\" \"registerListHandlers\")");
  evalHelper(list, env, false);
  std::stringstream bench("(load \"./bench.so\" \"registerBenchHandlers\")");
  evalHelper(bench, env, false);
  std::stringstream comp("(load \"./compiler.so\" \"registerCompilerHandlers\")");
  evalHelper(comp, env, false);
  std::stringstream debug("(load \"./debug.so\" \"registerDebugHandlers\")");
  evalHelper(debug, env, false);

  bool verbose = true;
  if(argc >= 3) 
    if(std::string("-q").compare(argv[2]) == 0)
      verbose = false;

  //loadFile("lib/core.cl", env, verbose);    

  if(argc >= 2)
    loadFile(argv[1], env, verbose);


  while(true)
    {
      line_read = readline ("elisp> ");
      
      if(line_read == NULL) {
        std::cout << std::endl << "Goodbye." << std::endl;
        break;
      }

      if (line_read && *line_read) {
          in = std::string(line_read);
          add_history (line_read);
      } 
      size_t posc = in.find(";");
      size_t posp = in.find("(");
      if(posc < posp)
        {
          std::cout << "skip comment" << std::endl;
					continue;
        }
 
      curLine += " " +  in;
      std::stringstream cl(curLine);
      if(evalHelper(cl, env, verbose))
        curLine = "";
      free(line_read);
    }
  
  return 0;
}
