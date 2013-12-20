#ifndef _PRIV_QUEUE_H
#define _PRIV_QUEUE_H
#include "eif_utils.hpp"
#include "spsc.hpp"

typedef spsc<call_data*> priv_queue_inner;

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

  bool synced;
  priv_queue_inner q;
  bool dirty;

  // GC interaction
public:
  void mark (marker_t mark);

private:
  int lock_depth;

};


#endif // _PRIV_QUEUE_H
