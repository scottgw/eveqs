#include "eveqs.h"
#include "internal.hpp"
#include "eif_macros.h"
#include "eif_scoop.h"

processor::processor(spid_t _pid) :
  pid(_pid), has_backing_thread (false)
{
}

void
processor::process_priv_queue(priv_queue_t pq)
{
  for (;;)
    {
      call_data *call;
      pq->q.pop((void *&)call);

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
processor::spawn()
{
  // FIXME: start up a thread via the eif_threads API
}

void
processor::application_loop()
{
  for (;;)
    {
      priv_queue_t pq;
      qoq.pop(pq);

      process_priv_queue (pq);
    }
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
