#ifndef _QUEUE_CACHE_H_
#define _QUEUE_CACHE_H_
#include <unordered_map>
#include <algorithm>
#include <stack>
#include <vector>
#include "private_queue.hpp"

typedef std::unordered_map <processor*, priv_queue*> queue_map;

class queue_cache {
public:
  queue_cache(processor* o) :
    owner (o),
    subs(),
    subs_pops(),
    base_map(),
    maps(),
    maps_pops()
  {
    subs.push_back(o);
    maps.push_back(&base_map);
  }

public:
  priv_queue*
  operator[] (processor *supplier)
  {
    for (const auto &m : maps)
      {
	auto found_it = m->find (supplier);
	if (found_it != m->end() && found_it->second->is_locked())
	  {
	    return found_it->second;
	  }
      }

    priv_queue *pq = new priv_queue (owner, supplier);
    base_map [supplier] = pq;
    return pq;
  }

  bool
  has_locked (processor *proc)
  {
    for (const auto &m : maps)
      {
	auto found_it = m->find (proc);
	if (found_it != m->end() && found_it->second->is_locked())
	  {
	    return true;
	  }
      }

    return false;
  }

  bool
  has_subordinate (processor *proc)
  {
    return std::count(subs.begin(), subs.end(), proc) > 0;
  }
  
public:
  void
  push (queue_cache* other)
  {
    maps_pops.push (other->maps.size());
    for (auto m : other->maps)
      {
	maps.push_back (m);
      }

    subs_pops.push (other->subs.size());
    for (auto s : other->subs)
      {
	subs.push_back (s);
      }
  }

  void
  pop ()
  {
    for (auto i = 0U; i < maps_pops.top(); i++)
      {
	maps.pop_back();
      }
    maps_pops.pop();

    for (auto i = 0U; i < subs_pops.top(); i++)
      {
	subs.pop_back();
      }
    subs_pops.pop();
  }

public:
  void
  mark (marker_t mark)
  {
    for (auto &pq_pair : base_map)
    {
      priv_queue *pq = pq_pair.second;
      pq->mark (mark);
    }
  }

private:
  processor *owner;
  std::vector<processor*> subs;
  std::stack<uint32_t> subs_pops;

  queue_map base_map;
  std::vector<queue_map*> maps;
  std::stack<uint32_t> maps_pops;
};

#endif // _QUEUE_CACHE_H_
