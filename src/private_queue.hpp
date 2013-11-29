#ifndef _PRIV_QUEUE_H
#define _PRIV_QUEUE_H

#include <tbb/concurrent_queue.h>

typedef tbb::concurrent_bounded_queue <void*> priv_queue_inner;

class processor;
typedef class processor* processor_t;

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

typedef class priv_queue* priv_queue_t;

#endif // _PRIV_QUEUE_H
