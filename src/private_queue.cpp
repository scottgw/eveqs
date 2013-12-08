#include "eveqs.h"
#include "internal.hpp"
#include "processor.hpp"
#include "private_queue.hpp"
#include "eif_lock.hpp"

priv_queue::priv_queue (processor_t *_client, processor_t *_supplier) :
  client (_client), supplier (_supplier), q()
{
  synced = false;
}

void
priv_queue::lock()
{
  supplier->qoq.push(this);
  synced = false;
}

void
priv_queue::register_wait()
{
  supplier->register_wait(client);
  synced = false;
}

void
priv_queue::log_call(void *data)
{
  call_data *call = (call_data*) data;
  bool will_sync = call_data_sync_pid (call) != NULL_PROCESSOR_ID;

  q.push (call);

  if (will_sync)
    {
      client->result_notify.wait();
    }

  synced = will_sync;
}

void
priv_queue::unlock()
{
  call_data *call = NULL;
  q.push (call);
  synced = false;
}
