#include <regex>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>


using namespace std;

int main()
{
		stringstream ss;
    ss << ",a";
    regex rea(ss.str()); 
		ss.str("");
    ss << ",b";
    regex reb(ss.str()); 
    string s { "(- ,a ,b)" };
    string ca { "(+ 3 1)" };
    string cb { "(- 6 4)" };

	 s = regex_replace(s, rea, ca);
	 s = regex_replace(s, reb, cb);
   std::cout << s << std::endl;
	return 0;
}
