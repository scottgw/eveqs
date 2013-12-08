#include "eif_block_token.hpp"
#include "eif_lock.hpp"
#include "eveqs.h"
#include "global.hpp"
#include "internal.hpp"
#include "private_queue.hpp"
#include "processor.hpp"
#include <eif_posix_threads.h>
#include <eif_threads.h>
#include <stdarg.h>

processor::processor(spid_t _pid,
                     bool _has_backing_thread,
                     void* _parent_obj) :
  has_backing_thread (_has_backing_thread),
  pid(_pid),
  parent_obj (_parent_obj)
{
  session_id = 0;
}

void
processor::process_priv_queue(priv_queue_t *pq)
{
  for (;;)
    {
      call_data *call;

      pq->q.pop(call);

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
  eif_lock lock (notify_mutex);
  uint32_t old_session_id = session_id;
  notify_cv.wait (lock.unique, [&](){return session_id == old_session_id;});
}

void
processor::notify_next(processor_t *current_client)
{
  eif_lock lock (notify_mutex);
  notify_cv.notify_all();
}

void
processor::application_loop()
{
  for (;;)
    {
      priv_queue_t *pq;

      qoq.pop(pq);

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
  qoq.push (NULL);
}
