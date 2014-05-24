#pragma once
#include <map>
#include <vector>
#include <string>
#include <functional>
class Sexp;
class Cell;

template <typename Key, typename Val>
class Env
{
public:
  static std::map<Key,Val> func;
  static std::map<Key, std::function<Val(Sexp* sexp, Env& env)> > evalHandlers;
  //private:
  std::vector<std::map<Key,Val>* > envs;
 private:
  static std::map<Key,Val> top;
public:
  Val& operator[] (const Key& k);
  Val& operator[] (Key& k);
  typename std::map<Key,Val>::iterator find(const Key& k);
  typename std::map<Key,Val>::iterator end() const;
  Env();
  void addEnvMap(std::map<Key, Val>* env);
  void removeEnv();

  template<typename K, typename V>
    friend std::ostream& operator<< (std::ostream& stream, const Env<K, V>& env);

};
