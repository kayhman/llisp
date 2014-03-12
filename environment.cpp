#include "environment.h"
#include "cell.h"

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
 typename std::map<Key,Val>::iterator Env<Key, Val>::find (const Key& k)
{
  for(auto envIt = envs.rbegin() ; envIt != envs.rend() ; envIt++)
    {
       typename std::map<Key,Val>::iterator lIt = (*envIt)->find(k);
	if(lIt != (*envIt)->end())
	  return lIt;
    }
  return this->top.end();
}

template <typename Key, typename Val>
typename std::map<Key,Val>::iterator Env<Key, Val>::end() const
{
  return this->top.end();
}

template<typename K, typename V> std::map<K, V> Env<K,V>::top;
template<typename K, typename V> std::map<K, std::function<V(Sexp* sexp, Env<K,V>& env)> > Env<K,V>::evalHandlers;

template class  Env<std::string,std::shared_ptr<Cell> >;
