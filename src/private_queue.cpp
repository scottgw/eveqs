#include "eveqs.h"
#include "internal.hpp"
#include "processor.hpp"
#include "private_queue.hpp"
#include "eif_macros.h"
#include "eif_scoop.h"

#define PRIV_QUEUE_CAPACITY 1024

priv_queue::priv_queue (processor_t _client, processor_t _supplier) :
  client (_client), supplier (_supplier), q()
{
  synced = false;
  q.set_capacity (PRIV_QUEUE_CAPACITY);
}

void
priv_queue::lock()
{
  supplier->qoq.push(this);
}

void
priv_queue::register_wait()
{
  supplier->register_wait(client);
}

void
priv_queue::log_call(void *data)
{
  call_data *call = (call_data*) data;
  bool will_sync = call_data_sync_pid (call) != NULL_PROCESSOR_ID;
  q.push (call);

  if (will_sync)
    {
      client->wait();
    }

  synced = will_sync;
}

void
priv_queue::unlock()
{
  q.push (NULL);
}
