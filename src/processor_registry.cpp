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
#include "internal.hpp"
#include "processor.hpp"
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <eif_macros.h>
#include <eif_scoop.h>
#include <iostream>
#include <mutex>
#include <stdlib.h>

#define MAX_PROCS 1024

processor_registry registry;

processor_registry::processor_registry () :
  free_pids (MAX_PROCS)
{
  for (spid_t i = 1; i < MAX_PROCS; i++)
    {
      free_pids.enqueue (i);
    }

  for (spid_t i = 0; i < MAX_PROCS; i++)
    {
      procs [i] = NULL;
    }

  used_pids.add(0);

  processor *root_proc = new processor(0);
  root_proc->has_client = true;

  procs[0] = root_proc;

  // end of life notification
  all_done = false;

  // GC
  is_marking = false;
}

processor*
processor_registry::create_fresh (void* obj)
{
  spid_t pid = 0;
  processor *proc;

  bool success = free_pids.dequeue (pid);
  assert (success & "There were no available PIDs");

  proc = new processor(pid, false);
  procs[pid] = proc;
  RTS_PID(obj) = pid;

  used_pids.add (pid);

  return proc;
}

processor*
processor_registry::operator[] (spid_t pid)
{
  assert (used_pids.has (pid));
  processor *proc = procs[pid];
  assert (proc & "processor_registry: processor retreived was NULL");
  return proc;
}

void
processor_registry::return_processor (processor *proc)
{
  spid_t pid = proc->pid;
  if (used_pids.erase (pid))
    {
      delete proc;
      procs [pid] = NULL;

      if (used_pids.size() == 0)
	{
	  std::unique_lock<std::mutex> lock(all_done_mutex);
	  all_done = true;
	  all_done_cv.notify_one();
	}

      // pid 0 is special so we don't recycle that one.
      if (pid)
	{
	  free_pids.enqueue (pid);
	}
    }
  else
    {
      assert (0 & "return_pid: shouldn't be here");
    }
}

// GC activities
void
processor_registry::enumerate_live ()
{
  for (spid_t i = 0; i < MAX_PROCS ; i++)
    {
      if (used_pids.has (i))
	{
	  processor* proc = (*this) [i];
	  
	  if (proc->has_client)
	    {
	      eif_mark_live_pid (proc->pid);
	    }
	}
    }
}

// use cas here for operations on is_marking
void
processor_registry::mark_all (marker_t mark)
{
  bool f = false;

  if (is_marking.compare_exchange_strong(f, true))
    {
      for (spid_t i = 0; i < MAX_PROCS ; i++)
	{
	  if (used_pids.has (i))
	    {
	      processor *proc = (*this) [i];
	      proc->mark (mark);
	    }
	}
      is_marking = false;
    }
}


void
processor_registry::unmark (spid_t pid)
{
  // This is a callback from the GC, which will notify us
  // of all unused processors, even those that have already been
  // shutdown, but still have a thread of execution.
  // To avoid double free/shutdown we check first to see if they're
  // still active.
  if (used_pids.has (pid))
    {
      processor_t *proc = (*this) [pid];
      proc->shutdown();
    }
}

void
processor_registry::wait_for_all()
{
  processor *root_proc = (*this)[0];

  // Clearing all the roots that point at
  // the root processor. It can now only be held
  // alive by references from other processors.
  // reclaim();
  root_obj = NULL;
  except_mnger = NULL;
  scp_mnger = NULL;
  rt_type_set = NULL;

  root_proc->has_client = false;
  root_proc->application_loop();
  return_processor (root_proc);

  while(!all_done)
    {
      eif_lock lock (all_done_mutex);
      all_done_cv.wait_for(lock,
			   std::chrono::milliseconds(200),
			   [&](){return all_done;});
    }
}

void
call_on (spid_t client_pid, spid_t supplier_pid, void* data)
{
  processor_t *client = registry [client_pid];
  processor_t *supplier = registry [supplier_pid];
  priv_queue_t *pq = client->find_queue_for (supplier);

  if (!supplier->has_backing_thread)
    {
      pq->lock();
      pq->log_call (data);
      pq->unlock();

      supplier->spawn();
      supplier->startup_notify.wait(NULL);
    }
  else
    {
      pq->log_call (data);
    }
}
