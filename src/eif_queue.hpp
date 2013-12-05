#ifndef _EIF_QUEUE_H
#define _EIF_QUEUE_H
#include <tbb/concurrent_queue.h>
#include "eif_macros.h"

#define SPIN 4

template <class V>
void
eif_push(tbb::concurrent_bounded_queue<V> &q, const V &val)
{
  for (int i = 0; i < SPIN; i++)
    {
      if (q.try_push(val))
        {
          return;
        }
    }

  EIF_ENTER_C;
  q.push(val);
  EIF_EXIT_C;
  RTGC;
}


template <class V>
void
eif_pop(tbb::concurrent_bounded_queue<V> &q, V &val)
{
  for (int i = 0; i < SPIN; i++)
    {
      if (q.try_pop(val))
        {
          return;
        }
    }

  EIF_ENTER_C;
  q.pop(val);
  EIF_EXIT_C;
  RTGC;
}

#endif // _EIF_QUEUE_H

