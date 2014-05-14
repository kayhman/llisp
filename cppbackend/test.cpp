#include <iostream>

class Sexp
{};

class Env
{};


Sexp* boubou(void* s, void* env)
{
	return 0;
}


int main()
{
	Sexp s;
	Env env;

	boubou(&s, &env);
	return 0;
}
