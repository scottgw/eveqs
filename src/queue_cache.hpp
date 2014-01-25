#ifndef _QUEUE_CACHE_H_
#define _QUEUE_CACHE_H_
#include <unordered_map>
#include <stack>
#include <set>
#include <vector>
#include "private_queue.hpp"

typedef std::vector<priv_queue*> queue_stack;

class queue_cache {
public:
  queue_cache(processor* o) :
    owner (o),
    sub_map(),
    sub_stack(),

    queue_map(),
    lock_stack()
  {
    sub_map[o] = 1;
  }


private:
  processor *owner;
  // The scheme for tracking the locks is to now use maps to stacks (or counts
  // for subordinates) to provide a mechanism that can have efficient
  // lookup, and still be able to push/pop these values.
  //
  // The goalis to first have efficient priv_queue lookup, as that is a very
  // common operation, while the push/pop operations are somewhat more expensive.
  std::unordered_map<processor*, uint32_t> sub_map;
  std::stack<std::set<processor*>> sub_stack;

  std::unordered_map <processor*, queue_stack> queue_map;
  std::stack<std::set<processor*>> lock_stack;

public:
  priv_queue*
  operator[] (processor *supplier)
  {
    const auto found_it = queue_map.find (supplier);
    priv_queue *pq;
    if (found_it != queue_map.end())
      {
	auto &stack = found_it->second;
	if (stack.empty())
	  {
	    stack.emplace_back (new priv_queue(owner, supplier));
	  }
	pq = stack.back();
      }
    else
      {
	auto res = queue_map.emplace (supplier, queue_stack());
	auto &stack = res.first->second;
	stack.emplace_back (new priv_queue(owner, supplier));
	pq = stack.back();
      }

    return pq;
  }

  bool
  has_locked (processor *proc) const
  {
    auto found_it = queue_map.find (proc);
    if (found_it != queue_map.end())
      {
	auto &stack = found_it->second;
	return !stack.empty() && stack.back()->is_locked();
      }

    return false;
  }

  bool
  has_subordinate (processor *proc) const
  {
    const auto res = sub_map.find (proc);
    return res != sub_map.end() && res->second > 0;
  }
  
public:
  void
  push (const queue_cache* other)
  {
    std::set <processor*> new_locks;
    for (const auto pair : other->queue_map)
      {
	const auto supplier = pair.first;
	auto &stack = pair.second;

	if (!stack.empty() && stack.back()->is_locked())
	  {
	    priv_queue *pq = stack.back();
	    new_locks.insert (supplier);

	    if (queue_map.find (supplier) == queue_map.end())
	      {
		queue_map [supplier] = queue_stack();
	      }
	    queue_map [supplier].emplace_back (pq);
	  }
      }
    lock_stack.push (new_locks);


    std::set <processor*> new_subs;
    for (const auto pair : other->sub_map)
      {
	const auto supplier = pair.first;
	const auto count = pair.second;

	if (count > 0)
	  {
	    new_subs.insert (supplier);

	    if (sub_map.find (supplier) == sub_map.end())
	      {
		sub_map [supplier] = 0;
	      }
	    sub_map [supplier]++;
	  }
      }
    sub_stack.push (new_subs);
  }

  void
  pop ()
  {
    const auto newest_locks = lock_stack.top();
    for (const auto &supplier : newest_locks)
      {
	queue_map[supplier].pop_back();
      }
    lock_stack.pop();

    auto newest_subs = sub_stack.top();
    for (const auto &supplier : newest_subs)
      {
	sub_map[supplier]--;
      }
    sub_stack.pop();
  }

public:
  void
  mark (marker_t mark)
  {
    for (const auto &pair : queue_map)
    {
      auto &stack = pair.second;
      for (auto &pq : stack)
	{
	  pq->mark (mark);
	}
    }
  }
};

#endif // _QUEUE_CACHE_H_
