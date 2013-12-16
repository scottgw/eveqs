#include "eif_utils.hpp"
#include "eveqs.h"
#include "global.hpp"
#include "internal.hpp"
#include "private_queue.hpp"
#include "processor.hpp"
#include <atomic>
#include <eif_posix_threads.h>
#include <eif_threads.h>
#include <stdarg.h>

std::atomic<int> active_count = ATOMIC_VAR_INIT (0);

processor::processor(spid_t _pid,
                     bool _has_backing_thread,
                     void* _parent_obj) :
  executing_call (NULL),
  has_backing_thread (_has_backing_thread),
  pid(_pid),
  parent_obj (_parent_obj)
{
  active_count++;
}

void
processor::process_priv_queue(priv_queue_t *pq)
{
  for (;;)
    {
      pq->q.pop(executing_call);

      if (executing_call == NULL)
        {
          return;
        }

      eif_try_call (executing_call);

      if (call_data_sync_pid (executing_call) != NULL_PROCESSOR_ID)
        {
          // We've processed the call so notify the client
          // that their result is ready.
          pq->client->result_notify.wake();
        }

      free (executing_call);
      executing_call = NULL;
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
  waiter = proc;
  notify_cv.wait (lock, [&](){return waiter != proc;});
}

void
processor::notify_next(processor_t *current_client)
{
  eif_lock lock (notify_mutex);
  waiter = current_client;
  notify_cv.notify_all();
}

void
processor::application_loop()
{
  for (;;)
    {
      priv_queue_t *pq;

      // Triggering the collection happens when all
      // processors are idle. This is sufficient for
      // program termination, but not sufficient for
      // freeing threads to let new ones take their
      // place.
      if (--active_count == 0)
        {
          plsc();
        }

      qoq.pop(pq);

      if (pq)
        {
          active_count++;
          has_client = true;

          process_priv_queue (pq);
          notify_next (pq->client);

          has_client = false;
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


void
processor::mark(marker_t mark)
{
  if (executing_call)
    {
      mark_call_data (mark, executing_call);
    }

  for (auto &pq : queue_cache)
    {
      pq.second->mark (mark);
    }
}
