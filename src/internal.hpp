#ifndef _INTERNAL_HPP
#define _INTERNAL_HPP
#include <stack>
#include <tbb/concurrent_queue.h>
#include <unordered_map>
#include <vector>
#include <future>
#include "eveqs.h"

#define NULL_PROCESSOR_ID -1

class priv_queue;
typedef class priv_queue* priv_queue_t;

class processor;
typedef class processor* processor_t;

typedef tbb::concurrent_bounded_queue <void*> priv_queue_inner;

class priv_queue
{
public:
  priv_queue (processor_t, processor_t);
  processor_t client;
  processor_t supplier;
  
  void lock();
  void log_call(void*);
  void unlock();

  priv_queue_inner q;
};

class req_grp : public std::vector<priv_queue_t>
{
public:
  req_grp();

  void add(processor_t, processor_t);
  void lock();
  void unlock();
private:
  bool sorted;
};

typedef tbb::concurrent_bounded_queue <priv_queue_t> qoq_t;
typedef tbb::concurrent_bounded_queue <void*> notifier_t;

class processor
{
public:
  processor(spid_t);
  priv_queue_t find_queue_for(processor_t);

  /* main loop */
  void application_loop();

  /* Request group stack operations */
  /* We have to maintain a stack of the request groups because
     I'd like to maintain compatibility with the code generation
     as it stands now.
     Their existing implementation also maintains a sort of stack
     like this */
  void lock_req_grp ();
  void add_to_req_grp (processor_t);
  void push_new_req_grp ();
  void unlock_req_grp();
  void pop_req_grp();

  /* waiting and waking for query results */
  void wake();
  void wait();

public:
  qoq_t qoq;
  spid_t pid;
private:
  notifier_t notifier;
  void process_priv_queue(priv_queue_t);

  std::stack <req_grp> req_grp_stack;
  std::unordered_map <processor_t, priv_queue_t> queue_cache;
};


processor_t
processor_get (spid_t);
#endif /* __INTERNAL_HPP*/
