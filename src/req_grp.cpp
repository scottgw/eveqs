#include <algorithm>
#include "eveqs.h"
#include "req_grp.hpp"
#include "processor.hpp"

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
