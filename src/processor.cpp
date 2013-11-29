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
processor::application_loop()
{
  for (;;)
    {
      priv_queue_t pq;

      // Indicating that we're blocking.
      eif_exit_eiffel_code();
      qoq.pop(pq);
      eif_enter_eiffel_code();
      // Unblock!

      process_priv_queue (pq);
    }
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
processor::lock_req_grp()
{
  req_grp_stack.top ().lock();
}

void
processor::add_to_req_grp (processor_t supplier)
{
  req_grp_stack.top ().add(this, supplier);
}

void
processor::push_new_req_grp ()
{
  req_grp_stack.emplace ();
}

void
processor::unlock_req_grp()
{
  req_grp_stack.top ().unlock();
}

void
processor::pop_req_grp()
{
  unlock_req_grp();
  req_grp_stack.pop();
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
  notifier.pop (dummy);
}
