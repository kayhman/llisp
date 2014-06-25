#include <iostream>
#include <fstream>
#include <regex>
#include <string>
/*
int main ()
{
  std::string s ("this subject has a submarine as a subsequence");
  std::smatch m;
  std::regex e ("\\b(sub)([^ ]*)");   // matches words beginning by "sub"

  std::cout << "Target sequence: " << s << std::endl;
  std::cout << "Regular expression: /\\b(sub)([^ ]*)/" << std::endl;
  std::cout << "The following matches and submatches were found:" << std::endl;

  while (std::regex_search (s,m,e)) {
    for (auto x:m) std::cout << x << " ";
    std::cout << std::endl;
    s = m.suffix().str();
  }

  return 0;
}
*/

int main ()
{
  std::string s ("(lapin un (pierre deux 3) (paul et miquelon)) ");
  std::string s2 (" lapin  deux trois");

  std::regex sexp ("\\s*(\\(.*\\))\\s*");
  std::regex word ("\\s*(\\w+)\\s*");

 std::smatch sm;    // same as std::match_results<string::const_iterator> sm;

  while (std::regex_search (s2,sm,word)) {
    for (auto x:sm) std::cout << x << " ";
    std::cout << std::endl;
    s2 = sm.suffix().str();
  }

  while (std::regex_search (s,sm,sexp)) {
    for (auto x:sm) std::cout << x << " ";
    std::cout << std::endl;
    s = sm.suffix().str();
  }


 
  std::regex_match (s,sm,sexp);
  std::cout << "string object with " << sm.size() << " matches\n";

  std::regex_match (s2,sm,word);
  std::cout << "string object with " << sm.size() << " matches\n";


  std::cout << "the matches were: " << std::endl;
  for (unsigned i=0; i<sm.size(); ++i) {
    std::cout << "[ " << sm[i] << " ] " << std::endl;
  }

  std::cout << std::endl;

  return 0;
}

/*
int main(int argc, char* argv[])
{
	std::ifstream in(argv[1]);

	while(!in.eof())
	{
		std::string st;
		in >> st;
		std::cout << "word " << st << std::endl;
	}
	return 0;
}*/
