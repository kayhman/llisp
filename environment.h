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


template <typename Key, typename Val>
Env<Key, Val>::Env()
{
  envs.push_back(&top);
}

template <typename Key, typename Val>
void Env<Key, Val>::addEnvMap(std::map<Key, Val>* env)
{
  this->envs.push_back(env);
}

template <typename Key, typename Val>
void Env<Key, Val>::removeEnv()
{
  this->envs.pop_back();
}

template <typename Key, typename Val>
Val& Env<Key, Val>::operator[] (const Key& k)
{
  for(auto envIt = envs.rbegin() ; envIt != envs.rend() ; envIt++)
    if((*envIt)->find(k) != (*envIt)->end())
 	return (**envIt)[k];
   return (*envs.back())[k];
}

template <typename Key, typename Val>
Val& Env<Key, Val>::operator[] (Key& k)
{
  for(auto envIt = envs.rbegin() ; envIt != envs.rend() ; envIt++)
    if((*envIt)->find(k) != (*envIt)->end())
 	return (**envIt)[k];
   return (*envs.back())[k];
}

template <typename Key, typename Val>
bool Env<Key, Val>::find (const Key& k)
{
  for(auto envIt = envs.rbegin() ; envIt != envs.rend() ; envIt++)
    if((*envIt)->find(k) != (*envIt)->end())
	return true;
  return false;
}
