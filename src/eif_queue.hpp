#ifndef _EIF_QUEUE_H
#define _EIF_QUEUE_H
#include <tbb/concurrent_queue.h>
#include "qoq.hpp"
#include "eif_block_token.hpp"

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

#endif // _EIF_QUEUE_H

