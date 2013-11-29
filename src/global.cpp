#include <stdlib.h>
#include <mutex>
#include "eveqs.h"
#include "internal.hpp"
#include "processor.hpp"
#include "eif_macros.h"

#define MAX_PROCS 1024

union proc_or_free_id {
  processor_t proc;
  spid_t next_free_id;
};

std::mutex mutex;
proc_or_free_id *proc_list = 0;

// Start at 1 because it is for root (I believe)
spid_t free_id = 1;

void
processor_init()
{
  proc_list = (proc_or_free_id*) malloc (MAX_PROCS * sizeof(proc_or_free_id));

  proc_list[0].proc = new processor(0);

  for (int i = 1; i < MAX_PROCS; i++)
    {
      proc_list[i].next_free_id = (i + 1) % MAX_PROCS;
    }
}

void
processor_fresh (void *obj)
{
  if (proc_list == 0)
    {
      processor_init();
    }

  spid_t id;
  {
    std::lock_guard <std::mutex> lock (mutex);
    id = free_id;
    free_id = proc_list[id].next_free_id;
  }

  // It is legit to release the lock here because the only
  // way someone could access proc_list[id] is if we free
  // the id again.
  proc_list[id].proc = new processor(id, false, obj);
  RTS_PID(obj) = id;
}

void
processor_free_id (processor_t proc)
{
  std::lock_guard <std::mutex> lock (mutex);
  proc_list[proc->pid].next_free_id = free_id;
  free_id = proc->pid;
}

processor_t
processor_get (spid_t pid)
{
  return proc_list[pid].proc;
}


void
call_on (spid_t client_pid, spid_t supplier_pid, void* data)
{
  processor_t client = processor_get (client_pid);
  processor_t supplier = processor_get (supplier_pid);

  if (!supplier->has_backing_thread)
    {
      supplier->spawn();
      // FIXME: add inline acquisition of private queue,
      // locking, logging, and unlocking.
    }

  // FIXME: log call
}
