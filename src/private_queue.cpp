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

#include "eveqs.h"
#include "internal.hpp"
#include "processor.hpp"
#include "private_queue.hpp"
#include "eif_utils.hpp"

priv_queue::priv_queue (processor_t *_client, processor_t *_supplier) :
  spsc<call_data*>(),
  client (_client),
  supplier (_supplier),
  dirty (false),
  synced (false),
  lock_depth(0)
{
}

void
priv_queue::lock()
{
  if (lock_depth == 0)
    {
      supplier->qoq.push(this);
      synced = false;
    }

  lock_depth++;
}

bool
priv_queue::is_locked()
{
  return lock_depth > 0;
}

bool
priv_queue::is_synced()
{
  return synced;
}

void
priv_queue::register_wait()
{
  client->my_token.register_supplier(supplier);
  synced = false;
}

void
priv_queue::log_call(call_data *call)
{
  bool will_sync = call_data_sync_pid (call) != NULL_PROCESSOR_ID;

  push (call);

  if (will_sync)
    {
      client->result_notify.wait(call);
    }

  synced = will_sync;
}

void
priv_queue::unlock()
{
  lock_depth--;

  if (lock_depth == 0)
    {
      push (NULL);
      synced = false;
    }
}

void
priv_queue::mark(marker_t mark)
{
  auto mark_call =
    [&](call_data* call)
      {
        if (call)
          {
            mark_call_data (mark, call);
          }
      };
  unsafe_map_ (mark_call);
}
