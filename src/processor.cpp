#include <stdarg.h>
#include "eveqs.h"
#include "internal.hpp"
#include "global.hpp"
#include "processor.hpp"
#include "private_queue.hpp"
#include "eif_posix_threads.h"
#include "eif_threads.h"
#include "eif_macros.h"
#include "eif_scoop.h"


processor::processor(spid_t _pid,
                     bool _has_backing_thread,
                     void* _parent_obj) :
  has_backing_thread (_has_backing_thread),
  pid(_pid),
  parent_obj (_parent_obj)
{
  session_id = 0;
  stub = node_allocator.allocate(1);
  mpscq_create (&qoq, stub);
}

void
processor::process_priv_queue(priv_queue_t *pq)
{
  for (;;)
    {
      call_data *call;

      pq->pop(call);

      if (call == NULL)
        {
          return;
        }

      eif_try_call (call);

      if (call_data_sync_pid (call) != NULL_PROCESSOR_ID)
        {
          // We've processed the call so notify the client
          // that their result is ready.
          pq->client->result_notify.wake();
        }

      eif_free_call (call);
    }
}


void
spawn_main(char* data, spid_t pid)
{
  processor_t *proc = processor_get(pid);
  proc->application_loop();
}


void
processor::spawn()
{
  eif_thr_create_with_attr_new ((char**)parent_obj, // No root object, if this is only
                                                    // passed to spawn_main this is OK
                                (void (*)(char* data, ...)) spawn_main,
                                pid, // Logical PID
                                EIF_TRUE, // We are a processor
                                NULL); // There are no attributes
}

void
processor::register_wait(processor_t *proc)
{
  EIF_ENTER_C;
  {
    std::unique_lock<std::mutex> lock (notify_mutex);
    uint32_t old_session_id = session_id;
    while (session_id == old_session_id)
      {
        notify_cv.wait (lock);
      }
  }
  EIF_EXIT_C;
  RTGC;
}

void
processor::notify_next(processor_t *current_client)
{
  EIF_ENTER_C;
  {
    std::unique_lock<std::mutex> lock (notify_mutex);
    notify_cv.notify_all();
  }
  EIF_EXIT_C;
  RTGC;
}

void
processor::application_loop()
{
  for (;;)
    {
      priv_queue_t *pq;

      qoq_pop(pq);

      if (pq)
        {
          session_id++;
          process_priv_queue (pq);
          notify_next (pq->client);
        }
      else
        {
          break;
        }

    }

  processor_free_id (this);
}

void
processor::deallocate(mpscq_node_t *node)
{
  node_allocator.deallocate(node, 1);
}

void
processor::qoq_push(void *val)
{
  mpscq_node_t * node = node_allocator.allocate(1);
  node->state = val;

  mpscq_push(&qoq, node);

  {
    std::unique_lock<std::mutex> lock(qoq_mutex);
    qoq_cv.notify_all();
  }
}

void
processor::qoq_pop(priv_queue * &pq)
{
  mpscq_node_t *node;

  for (int i = 0; i < 128; i++)
    {
      if ((node = mpscq_pop(&qoq)) != 0)
        {
          pq = (priv_queue *) node->state;
          pq->client->deallocate(node);
          return;
        }
    }


  EIF_ENTER_C;
  {
    std::unique_lock<std::mutex> lock(qoq_mutex);
    while ((node = mpscq_pop(&qoq)) == 0)
      {
        qoq_cv.wait(lock);
      }
  }
  EIF_EXIT_C;
  RTGC;

  pq = (priv_queue *) node->state;
  pq->client->deallocate (node);
}

priv_queue_t*
processor::find_queue_for(processor_t *supplier)
{
  auto queue_maybe = queue_cache.find (supplier);
  if (queue_maybe == queue_cache.end())
    {
      priv_queue_t *pq = new priv_queue (this, supplier);
      queue_cache[supplier] = pq;
      return pq;
    }
  else
    {
      return queue_maybe->second;
    }
}

void
processor::shutdown()
{
  qoq_push (NULL);
}
