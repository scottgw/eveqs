//
// EVE/Qs - A new runtime for the EVE SCOOP implementation
// Copyright (C) 2014 Scott West <scott.gregory.west@gmail.com>
//
// This file is part of EVE/Qs.
//
// EVE/Qs is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EVE/Qs is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EVE/Qs.  If not, see <http://www.gnu.org/licenses/>.
//

#include <algorithm>
#include "eveqs.h"
#include "req_grp.hpp"
#include "processor.hpp"

req_grp::req_grp(processor_t *_client) :
  vector<priv_queue_t*> (),
  client (_client),
  sorted (false)
{
}

void
req_grp::add(processor_t *supplier)
{
  priv_queue_t *pq = client->find_queue_for (supplier);
  push_back (pq);
}

void
req_grp::wait()
{
  for (auto &pq : *this)
    {
      pq->register_wait();
    }

  unlock();

  client->my_token.wait();
}

void
req_grp::lock()
{
  auto sort_func =  [](priv_queue_t *pq1, priv_queue_t *pq2) 
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
  for (auto it = rbegin(); it != rend(); ++it)
    {
      (*it)->unlock();
    }
}
