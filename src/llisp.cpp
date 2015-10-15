#include "parse.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <iostream>
#include <sstream>


int main(int argc, char* argv[])
{ 

  std::string in;
  std::string curLine;
  char* line_read = NULL;

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
      if(evalHelper(cl,  true))
        curLine = "";
      free(line_read);
    }
  
  return 0;
}
