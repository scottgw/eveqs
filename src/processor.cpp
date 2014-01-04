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
  my_token (this),
  executing_call (NULL),
  has_backing_thread (_has_backing_thread),
  pid(_pid),
  parent_obj (_parent_obj)
{
  active_count++;
}

// This is a modified RTE_T with no `start' label
#define RTE_T_QS              \
  saved_except = RTLA;        \
  exvect->ex_jbuf = &exenv;   \
  if (!setjmp(exenv)) {

void
processor::try_call (priv_queue_t *pq, call_data *call)
{
  // This commented section slows down some benchmarks by 2x. I believe
  // this is due to either some locking in the allocation routines (again)
  // or reloading the thread local variables often.

  // This is too slow (I think). Let's just memorize it once then reuse that.
  // if (!globals)
  //   {
  //     GTCX;
  //     globals = eif_globals;
  //   }

  // {
  //   eif_global_context_t *eif_globals = globals;

  //   EIF_REFERENCE EIF_VOLATILE saved_except = (EIF_REFERENCE) 0;
  //   RTEX;
  //   RTED;
  //   RTEV;
  //   RTE_T_QS;
  //   eif_try_call (call_data);
  //   RTE_E;
  //   pq->dirty = true;
  //   RTE_EE;
  // }


  // This just shows that the jmp_buf isn't the bottle-neck: this is fast.

  jmp_buf buf;
  
  if (!setjmp(buf))
    {
      eif_try_call (call);
    }
  else
    {
      pq->dirty = true;
    }
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

      try_call (pq, executing_call);
      // eif_try_call (executing_call);

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
  processor_t *proc = registry [pid];
  proc->application_loop();
  registry.return_processor (proc);
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
processor::register_notify_token (notify_token token)
{
  token_queue.push (token);
}

void
processor::notify_next(processor *client)
{
  auto n = token_queue.size();
  for (auto i = 0; i < 10 && !token_queue.empty(); i++)
    {
      auto token = token_queue.front();
      token_queue.pop();

      if (token.client() == client)
        {
          token_queue.push(token);
        }
      else
        {
          token.notify(client);
        }
    }
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
