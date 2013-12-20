#ifndef _PROCESSOR_H
#define _PROCESSOR_H
#include <memory>
#include <stack>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include "qoq.hpp"
#include "eif_utils.hpp"
#include "private_queue.hpp"
#include "req_grp.hpp"
#include "spsc.hpp"

typedef tbb::concurrent_bounded_queue <processor_t*> waiters_t;                 

typedef tbb::concurrent_bounded_queue <void*> notifier_queue;

struct notifier : notifier_queue {
  void wait ()
  {
    void* dummy;
    eif_pop(static_cast<notifier_queue*> (this), dummy);
  }

  void wake ()
  {
    void* dummy = NULL;
    eif_push(static_cast<notifier_queue*> (this), dummy);
  }
};

class processor
{
public:
  processor(spid_t _pid,
            bool _has_backing_thread = false,
            void* _parent_obj = NULL);

  // lifetime operations
  void application_loop();
  void shutdown();

  // queue cache
public:
  priv_queue_t* find_queue_for(processor_t*);

  // queue operations, move to private and offer clients
  // only pushing capabilities?
public:
  mpscq <priv_queue*> qoq;

  // separate argument stacks
public:
  std::stack <req_grp> group_stack;

  // registration for wait condition notification
public:
  void register_wait(processor_t *proc);
  void notify_next(processor_t *current_client);

  std::mutex notify_mutex;
  std::condition_variable notify_cv;
  processor *waiter;
  notifier wait_cond_notify;
  
  // GC interaction
public:
  bool has_client;   // activity flag for the GC
  call_data* executing_call;
  void mark (marker_t mark);

  // result notification
public:
  notifier result_notify;

  // Interoperability with the ES runtime
public:
  void spawn();
  bool has_backing_thread;
  spid_t pid;
  void* parent_obj;
  eif_global_context_t *globals;
  void try_call (priv_queue_t*, call_data*);


  // private stuff, no particular grouping
private:
  waiters_t waiters;
  void process_priv_queue(priv_queue_t*);
  std::unordered_map <processor_t*, priv_queue_t*> queue_cache;
};


#endif // _PROCESSOR_H
