#include "eveqs.h"
#include "internal.hpp"
#include "processor.hpp"
#include "private_queue.hpp"
#include "eif_block_token.hpp"

priv_queue::priv_queue (processor_t *_client, processor_t *_supplier) :
  client (_client), supplier (_supplier), q()
{
  synced = false;
  q = spsc_queue();
}


void
priv_queue::spsc_push(call_data* data)
{
  spsc_enqueue(q, data);
  {
    std::unique_lock<std::mutex> lock (mutex);
    cv.notify_one();
  }
}

void
priv_queue::pop(call_data* &data)
{
  for (int i = 0; i < 512; i++)
    {
      if (spsc_dequeue(q, &data))
        {
          return;
        }
    }

  {
    eif_block_token token;
    std::unique_lock<std::mutex> lock (mutex);
    while (!spsc_dequeue(q, &data))
      {
        cv.wait(lock); // _for(lock, std::chrono::milliseconds(50));
      }
  }
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

  spsc_push (call);

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
  spsc_push (call);
  synced = false;
}
