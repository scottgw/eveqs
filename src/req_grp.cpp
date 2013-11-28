#include <algorithm>
#include "eveqs.h"
#include "internal.hpp"

req_grp::req_grp() : vector<priv_queue_t> ()
{
  sorted = false;
}

void
req_grp::add(processor_t client, processor_t supplier)
{
  priv_queue_t pq = client->find_queue_for (supplier);
  push_back (pq);
}

void
req_grp::lock()
{
  auto sort_func =  [](priv_queue_t pq1, priv_queue_t pq2) 
  {
    return pq1->supplier->pid <= pq2->supplier->pid; 
  };

  if (!sorted)
    {
      std::sort (begin(), end(), sort_func);
    }

  sorted = true;

  for (auto &pq : *this)
    {
      pq->lock();
    }
}

void
req_grp::unlock()
{
  // Unlock in the opposite order that they were locked
  for (auto it = this->rbegin(); it != this->rend(); ++it)
    {
      (*it)->lock();
    }
}

extern "C"
void
req_grp_new (spid_t client_pid)
{
  processor_t client = processor_get (client_pid);
  client->push_new_req_grp();
}

extern "C"
void
req_grp_add_supplier (spid_t client_pid, spid_t supplier_pid)
{
  processor_t client = processor_get (client_pid);
  processor_t supplier = processor_get (supplier_pid);  
  client->add_to_req_grp (supplier);
}

extern "C"
void
req_grp_lock (spid_t client_pid)
{
  processor_t client = processor_get (client_pid);
  client->lock_req_grp();
}
