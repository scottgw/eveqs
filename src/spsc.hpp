#ifndef _SPSC_H
#define _SPSC_H
#include <mutex>
#include <condition_variable>
#include "spsc_queue.h"

class spsc
{
public:
  spsc()
  {
    q = spsc_queue();
  }

  void
  pop(call_data* &data)
  {
    for (int i = 0; i < 512; i++)
      {
        if (spsc_dequeue(q, &data))
          {
            return;
          }
      }

    {
      eif_lock lock (mutex);
      while (!spsc_dequeue(q, &data))
        {
          cv.wait(lock.unique);
        }
    }
  }

  void
  push(call_data* data)
  {
    spsc_enqueue(q, data);
    {
      std::unique_lock<std::mutex> lock (mutex);
      cv.notify_one();
    }
  }

private:
  spsc_queue_t *q;
  std::mutex mutex;
  std::condition_variable cv;
};

#endif // _SPSC_H
