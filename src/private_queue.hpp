#ifndef _PRIV_QUEUE_H
#define _PRIV_QUEUE_H
#include "eif_queue.hpp"
#include "spsc_queue.h"

typedef spsc_queue_t* priv_queue_inner;

class processor;
typedef class processor processor_t;

class priv_queue
{
public:
  priv_queue (processor_t*, processor_t*);
  processor_t* client;
  processor_t* supplier;
  
  void lock();
  void log_call(void*);
  void register_wait();
  void unlock();
  void spsc_push (call_data*);
  void pop(call_data* &data);

  bool synced;
  priv_queue_inner q;

  std::mutex mutex;
  std::condition_variable cv;

};


#endif // _PRIV_QUEUE_H
