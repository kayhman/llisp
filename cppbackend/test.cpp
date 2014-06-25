#include <iostream>

class Sexp
{};

class Env
{};



extern "C" Sexp* boubou(void* s, void* env);
extern "C" void* bubu;


int main()
{
	Sexp s;
	Env env;
	boubou(bubu, NULL);
	return 0;
}
