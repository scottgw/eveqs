#include "eveqs.h"
#include "internal.hpp"
#include "processor.hpp"
#include "private_queue.hpp"
#include "eif_macros.h"
#include "eif_scoop.h"

#define PRIV_QUEUE_CAPACITY 1024


void
priv_queue::spsc_push(call_data* data)
{
  std::unique_lock<std::mutex> lock (mutex);
  q.enqueue(data);
  cv.notify_one();
}

priv_queue::priv_queue (processor_t *_client, processor_t *_supplier) :
  client (_client), supplier (_supplier), q()
{
  synced = false;
  // q.set_capacity (PRIV_QUEUE_CAPACITY);
}

void
priv_queue::lock()
{
  eif_push (supplier->qoq, this);
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

  spsc_push (call);

  if (will_sync)
    {
      client->wait();
    }

  synced = will_sync;
}

void
priv_queue::unlock()
{
  call_data *call = NULL;
  spsc_push (call);
  synced = false;
}
