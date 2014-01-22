#ifndef _QUEUE_CACHE_H_
#define _QUEUE_CACHE_H_
#include <unordered_map>
#include <vector>
#include <stack>
#include "private_queue.hpp"

typedef std::unordered_map <processor*, priv_queue*> queue_map;
typedef std::vector <queue_map*> map_stack;

class queue_cache {
public:
  queue_cache(processor* o) :
    owner (o),
    pop_sizes(),
    maps(),
    base_map()
  {
    maps.push_back (&base_map);
  }

public:
  priv_queue*
  operator[] (processor *supplier)
  {
    queue_map* found_map = NULL;
    for (auto qmap : maps)
      {
	if (qmap->count (supplier))
	  {
	    found_map = qmap;
	    break;
	  }
      }

    if (found_map)
      {
	return (*found_map)[supplier];
      }
    else
      {
	priv_queue *pq = new priv_queue (owner, supplier);
	(*maps[0])[supplier] = pq;
	return pq;
      }
}
  
public:
  void
  push (queue_cache* other)
  {
    pop_sizes.push (other->maps.size());
    for (const auto other_map : other->maps)
      {
	maps.push_back (other_map);
      }
  }

  void
  pop ()
  {
    for (auto i = 0U; i < pop_sizes.top(); i++)
      {
	maps.pop_back ();
      }
    pop_sizes.pop();
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
  std::stack<uint32_t> pop_sizes;
  map_stack maps;
  queue_map base_map;
};

#endif // _QUEUE_CACHE_H_
