#pragma once
#include <map>
#include <vector>
#include <string>

class Sexp;
class Cell;

template <typename Key, typename Val>
class Env
{
public:
  static std::map<Key,Val> top;
  static std::map<Key, std::function<Val(Sexp* sexp, Env& env)> > evalHandlers;
private:
  std::vector<std::map<Key,Val>* > envs;
public:
  Val& operator[] (const Key& k);
  Val& operator[] (Key& k);
  bool find (const Key& k);
  Env();
  void addEnvMap(std::map<Key, Val>* env);
  void removeEnv();
};
