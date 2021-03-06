#include "environment.h"
#include "cell.h"

void* clos2 = NULL;

template <typename Key, typename Val>
Env<Key, Val>::Env()
{
  envs.push_back(&top);
}

template <typename Key, typename Val>
Env<Key, Val>::~Env()
{
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
    {
      auto it = (*envIt)->find(k);
      if(it != (*envIt)->end())
 	return it->second;
    }
      return (*envs.back())[k];
}

template <typename Key, typename Val>
Val& Env<Key, Val>::operator[] (Key& k)
{
  
  for(auto envIt = envs.rbegin() ; envIt != envs.rend() ; envIt++)
    {
      auto it = (*envIt)->find(k);
      if(it != (*envIt)->end())
	return it->second;
    }
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
typename std::map<Key,Val>::const_iterator Env<Key, Val>::end() const
{
    return this->top.end();
}


template <typename Key, typename Val>
std::ostream& operator<< (std::ostream& stream, const Env<Key, Val>& env)
  {
    for(auto eIt = env.envs.begin() ; eIt != env.envs.end() ; eIt++)
      {
	for(auto it = (*eIt)->begin() ; it != (*eIt)->end() ; it++)
	  {
	    std::cout << it->first << " : " << *(it->second) << std::endl;
	  }
      }

    for(auto it = env.func.begin() ; it != env.func.end() ; it++)
      {
	std::cout << "func : " << it->first << " : " << *(it->second) << " compiled? " << it->second->compiled << std::endl;
      }

    return stream;
  }

//template<typename K, typename V> std::map<K, V> Env<K,V>::top;
template<typename K, typename V> std::map<K, V> Env<K,V>::func;

template std::ostream& operator<< (std::ostream& stream, const Env<std::string, std::shared_ptr<Cell> >& env);
template class  Env<std::string,std::shared_ptr<Cell> >;
