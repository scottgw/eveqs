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

#include "eif_utils.hpp"
#include "eveqs.h"
#include "processor_registry.hpp"
#include "processor.hpp"

extern "C"
{
  // RTS_RC (o) - create request group for o
  void
  eveqs_req_grp_new (spid_t client_pid)
  {
    processor_t *client = registry [client_pid];
    client->group_stack.push(req_grp(client));
  }

  // RTS_RD (o) - delete chain (release locks?)
  void
  eveqs_req_grp_delete (spid_t client_pid)
  {
    processor_t *client = registry [client_pid];
    client->group_stack.top().unlock();
    client->group_stack.pop();
  }

  // RTS_RF (o) - wait condition fails
  void
  eveqs_req_grp_wait (spid_t client_pid)
  {
    processor_t *client = registry [client_pid];
    client->group_stack.top().wait();
  }

  // RTS_RS (c, s) - add supplier s to current group for c
  void
  eveqs_req_grp_add_supplier (spid_t client_pid, spid_t supplier_pid)
  {
    processor_t *client = registry [client_pid];
    processor_t *supplier = registry [supplier_pid];  
    client->group_stack.top().add (supplier);
  }

  // RTS_RW (o) - sort all suppliers in the group and get exclusive access
  void
  eveqs_req_grp_lock (spid_t client_pid)
  {
    processor_t *client = registry [client_pid];
    client->group_stack.top().lock();
  }

  //
  // Processor creation
  //

  // RTS_PA
  void
  eveqs_processor_fresh (void *obj)
  {
    registry.create_fresh (obj);
  }

  //
  // Call logging
  //

  void
  eveqs_call_on (spid_t client_pid, spid_t supplier_pid, void* data)
  {
    call_data *call = (call_data*) data;
    processor_t *client = registry [client_pid];
    processor_t *supplier = registry [supplier_pid];
    priv_queue_t *pq = client->find_queue_for (supplier);

    if (!supplier->has_backing_thread)
      {
	pq->lock();
	pq->log_call (call);
	pq->unlock();

	supplier->spawn();
	supplier->startup_notify.wait(NULL);
      }
    else
      {
	pq->log_call (call);
      }
  }


  int
  eveqs_is_synced_on (spid_t client_pid, spid_t supplier_pid)
  {
    processor_t *client = registry [client_pid];
    processor_t *supplier = registry [supplier_pid];  

    priv_queue_t *pq = client->find_queue_for (supplier);
    return pq->is_synced();
  }

  int
  eveqs_is_uncontrolled(spid_t client_pid, spid_t supplier_pid)
  {
    return 1;
  }


  //
  // Callback from garbage collector to indicate that the
  // processor isn't used anymore.
  //
  void
  eveqs_unmarked(spid_t pid)
  {
    registry.unmark (pid);
  }

  void
  eveqs_enumerate_live()
  {
    registry.enumerate_live();
  }

  void
  eveqs_wait_for_all()
  {
    registry.wait_for_all();
  }

  void
  eveqs_mark_all (marker_t mark)
  {
    registry.mark_all (mark);
  }
}
