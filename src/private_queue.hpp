#ifndef _PRIV_QUEUE_H
#define _PRIV_QUEUE_H
#include "eif_queue.hpp"
#include "spsc_queue.hpp"

typedef spsc_queue <call_data*> priv_queue_inner;

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
  void spsc_push (call_data *data);
  void pop(call_data* &data)
  {
    for (int i = 0; i < 16; i++)
      {
        if (q.dequeue(data))
          {
            return;
          }
      }

    while (!q.dequeue(data))
      {
        EIF_ENTER_C;
        std::unique_lock<std::mutex> lock (mutex);
        cv.wait_for(lock, std::chrono::milliseconds(100));
        EIF_EXIT_C;
        RTGC;
      }
  }

  bool synced;
  priv_queue_inner q;

  std::mutex mutex;
  std::condition_variable cv;

};


#endif // _PRIV_QUEUE_H
