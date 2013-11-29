#ifndef _PROCESSOR_H
#define _PROCESSOR_H
#include <stack>
#include <unordered_map>
#include "private_queue.hpp"
#include "req_grp.hpp"

typedef tbb::concurrent_bounded_queue <priv_queue_t> qoq_t;
typedef tbb::concurrent_bounded_queue <void*> notifier_t;

class processor
{
public:
  processor(spid_t _pid,
            bool _has_backing_thread = false,
            void* _parent_obj = NULL);
  priv_queue_t find_queue_for(processor_t);

  /* main loop */
  void application_loop();
  void spawn();

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
  bool has_backing_thread;
  qoq_t qoq;
  spid_t pid;

private:
  void* parent_obj;
  notifier_t notifier;
  void process_priv_queue(priv_queue_t);

  std::stack <req_grp> req_grp_stack;
  std::unordered_map <processor_t, priv_queue_t> queue_cache;
};


#endif // _PROCESSOR_H
