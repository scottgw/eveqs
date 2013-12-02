#define EIF_THREADS
#define EIF_POSIX_THREADS
#include <stdarg.h>
#include "eveqs.h"
#include "internal.hpp"
#include "global.hpp"
#include "processor.hpp"
#include "eif_posix_threads.h"
#include "eif_threads.h"
#include "eif_macros.h"
#include "eif_scoop.h"

processor::processor(spid_t _pid,
                     bool _has_backing_thread,
                     void* _parent_obj) :
  pid(_pid), has_backing_thread (_has_backing_thread), parent_obj (_parent_obj)
{
}

void
processor::process_priv_queue(priv_queue_t pq)
{
  for (;;)
    {
      call_data *call;

      // As we block we indicate that we're doing so
      eif_exit_eiffel_code();
      pq->q.pop((void *&)call);
      eif_enter_eiffel_code();
      // Unblock!

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

  processor_t proc = processor_get(pid);

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
processor::register_wait(processor_t proc)
{
  waiters.push(proc);
}

void
processor::notify_next(processor_t current_client)
{
  processor_t waiter;

  waiters.pop(waiter);

  if (waiter == current_client)
    {
      // If the waiters is now empty then we don't want to
      // push the current_client only to pop and wake it.
      if (waiters.size() == 0)
        {
          return;
        }

      waiters.push(waiter);
    }

  waiters.pop(waiter);
  waiter->wake();
}

void
processor::application_loop()
{
  for (;;)
    {
      priv_queue_t pq;

      eif_exit_eiffel_code();
      qoq.pop(pq);
      eif_enter_eiffel_code();

      if (pq)
        {
          process_priv_queue (pq);
        }
      else
        {
          break;
        }

      notify_next(pq->client);
    }

  printf("processor::application_loop freeing\n");
  processor_free_id (this);
}

priv_queue_t
processor::find_queue_for(processor_t supplier)
{
  auto queue_maybe = queue_cache.find (supplier);
  if (queue_maybe == queue_cache.end())
    {
      priv_queue_t pq = new priv_queue (this, supplier);
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
  qoq.push(NULL);
}

void
processor::wake()
{
  notifier.push(NULL);
}

void
processor::wait()
{
  void* dummy;
  EIF_ENTER_C;
  notifier.pop (dummy);
  EIF_EXIT_C;
}
