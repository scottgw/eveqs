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
  }

public:
  priv_queue*
  operator[] (processor *supplier)
  {
    queue_map* found_map = NULL;
    for (auto qmap : maps)
      {
	// We only want the private queues that are already locked
	if (qmap->count (supplier) && (*qmap)[supplier]->is_locked())
	  {
	    found_map = qmap;
	    break;
	  }
      }

    if (found_map)
      {
	return (*found_map)[supplier];
      }
    else if (base_map.count (supplier))
      {
	return base_map [supplier];
      }
    else
      {
	priv_queue *pq = new priv_queue (owner, supplier);
	base_map [supplier] = pq;
	return pq;
      }
  }

  bool
  has_locked (processor *proc)
  {
    for (auto qmap : maps)
      {
	if (qmap->count (proc) && (*qmap)[proc]->is_locked())
	  {
	    return true;
	  }
      }

    return base_map.count (proc) && base_map[proc]->is_locked();
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
