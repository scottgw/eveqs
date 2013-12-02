#ifndef _PROCESSOR_H
#define _PROCESSOR_H
#include <stack>
#include <unordered_map>
#include "private_queue.hpp"
#include "req_grp.hpp"

typedef tbb::concurrent_bounded_queue <priv_queue_t> qoq_t;
typedef tbb::concurrent_bounded_queue <processor_t> waiters_t;
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
  void shutdown();

  /* registration for wait condition notification */
  void register_wait(processor_t proc);
  void notify_next(processor_t current_client);

  /* Request group stack operations */
  /* We have to maintain a stack of the request groups because
     I'd like to maintain compatibility with the code generation
     as it stands now.
     Their existing implementation also maintains a sort of stack
     like this */
  // void lock_req_grp ();
  // void add_to_req_grp (processor_t);
  // void push_new_req_grp ();
  // void unlock_req_grp();
  // void pop_req_grp();

  /* waiting and waking for query results */
  void wake();
  void wait();

public:
  bool has_backing_thread;
  qoq_t qoq;
  spid_t pid;
  std::stack <req_grp> group_stack;

private:
  void* parent_obj;
  notifier_t notifier;
  waiters_t waiters;
  void process_priv_queue(priv_queue_t);

  std::unordered_map <processor_t, priv_queue_t> queue_cache;
};


#endif // _PROCESSOR_H
