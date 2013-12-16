#ifndef _EIF_UTILS_H
#define _EIF_UTILS_H
#include <mutex>
#include <tbb/concurrent_queue.h>
#include "eif_macros.h"

typedef EIF_REFERENCE marker_t(EIF_REFERENCE *);

void
mark_call_data(marker_t mark, call_data* call);

class eif_block_token
{
public:
  eif_block_token ()
  {
    EIF_ENTER_C;
  }

  ~eif_block_token()
  {
    EIF_EXIT_C;
    RTGC;
  }
};


class eif_lock : eif_block_token, public std::unique_lock <std::mutex>
{
public:
  eif_lock (std::mutex &mutex) :
    eif_block_token (),
    std::unique_lock <std::mutex> (mutex)
  {}
};

#define SPIN 64

template <class V>
void
eif_push(tbb::concurrent_bounded_queue<V> *q, const V &val)
{
  for (int i = 0; i < SPIN; i++)
    {
      if (q->try_push(val))
        {
          return;
        }
    }

  {
    eif_block_token token;
    q->push(val);
  }
}


template <class V>
void
eif_pop(tbb::concurrent_bounded_queue<V> *q, V &val)
{
  for (int i = 0; i < SPIN; i++)
    {
      if (q->try_pop(val))
        {
          return;
        }
    }

  {
    eif_block_token token;
    q->pop(val);
  }
}

#endif // _EIF_UTILS_H
