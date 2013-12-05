#define EIF_THREADS
#define EIF_POSIX_THREADS
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
  pid(_pid), has_backing_thread (_has_backing_thread), parent_obj (_parent_obj)
{
  qoq.set_capacity(1024);
  last_waiter = NULL;
}

void
processor::process_priv_queue(priv_queue_t *pq)
{
  for (;;)
    {
      call_data *call;

      // printf("%d waiting on priv queue for %d\n", pid, pq->client->pid);
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
          pq->client->wake();
        }

      eif_free_call (call);
    }
}


void
spawn_main(char* data, ...)
{
  va_list ap;
  va_start (ap, 1);
  spid_t pid = (spid_t)va_arg(ap, int);
  va_end (ap);

  processor_t *proc = processor_get(pid);

  proc->application_loop();
}


void
processor::spawn()
{
  eif_thr_create_with_attr_new ((char**)parent_obj, // No root object, if this is only
                                                    // passed to spawn_main this is OK
                                spawn_main,
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
    // printf("%d waiting on %d\n", proc->pid, pid);
    while (proc == this->last_waiter)
      {
        notify_cv.wait (lock);
      }
  }
  EIF_EXIT_C;
  // RTGC;
  // printf("%d waking from %d\n", proc->pid, pid);
}

void
processor::notify_next(processor_t *current_client)
{
  EIF_ENTER_C;
  {
    std::unique_lock<std::mutex> lock (notify_mutex);
    last_waiter = current_client;
    notify_cv.notify_all();
  }
  EIF_EXIT_C;
  // RTGC;
}

void
processor::application_loop()
{
  for (;;)
    {
      priv_queue_t *pq;

      // printf("%d waiting on qoq\n", pid);
      eif_pop(qoq, pq);

      if (pq)
        {
          // printf("%d processing queue for %d\n", pid, pq->client->pid);
          process_priv_queue (pq);
          notify_next (pq->client);
        }
      else
        {
          break;
        }

    }

  // printf("processor::application_loop freeing\n");
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
  priv_queue_t *dummy = NULL;
  eif_push(qoq, dummy);
}

void
processor::wake()
{
  processor_t *dummy = NULL;
  eif_push(notifier, dummy);
}

void
processor::wait()
{
  processor_t *dummy;
  eif_pop(notifier, dummy);
}
