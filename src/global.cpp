#include "eif_utils.hpp"
#include "eveqs.h"
#include "global.hpp"
#include "internal.hpp"
#include "processor.hpp"
#include <chrono>
#include <condition_variable>
#include <eif_macros.h>
#include <eif_scoop.h>
#include <mutex>
#include <stdlib.h>
#include <tbb/concurrent_hash_map.h>

#define MAX_PROCS 1024

union proc_or_free_id {
  processor_t *proc;
  spid_t next_free_id;
};

std::mutex mutex;
proc_or_free_id *proc_list = 0;


bool all_done = false;
std::mutex all_done_mutex;
std::condition_variable all_done_cv;


typedef tbb::concurrent_hash_map <spid_t, bool> pid_set_t;

pid_set_t used_pid_set;

// Start at 1 because it is for root (I believe)
spid_t free_id = 1;

void
processor_init()
{
  proc_list = (proc_or_free_id*) malloc (MAX_PROCS * sizeof(proc_or_free_id));

  proc_list[0].proc = new processor(0);
  proc_list[0].proc->has_client = true;

  for (int i = 1; i < MAX_PROCS; i++)
    {
      proc_list[i].next_free_id = (i + 1) % MAX_PROCS;
    }

  {
    pid_set_t::const_accessor result;
    used_pid_set.insert(result, 0);
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

  {
    pid_set_t::const_accessor result;
    used_pid_set.insert(result, id);
  }

  // It is legit to release the lock here because the only
  // way someone could access proc_list[id] is if we free
  // the id again.
  proc_list[id].proc = new processor(id, false, obj);
  RTS_PID(obj) = id;
}

void
processor_free_id (processor_t *proc)
{
  spid_t pid = proc->pid;

  {
    std::lock_guard <std::mutex> lock (mutex);
    proc_list[pid].next_free_id = free_id;
    free_id = pid;
  }

  delete proc;

  used_pid_set.erase(pid);

  if (used_pid_set.size() == 0)
    {
      std::unique_lock<std::mutex> lock(all_done_mutex);
      all_done = true;
      all_done_cv.notify_one();
    }
}

void
processor_enumerate_live ()
{
  for (auto &pid : used_pid_set)
    {
      processor* proc = processor_get (pid.first);
      
      if (proc->has_client)
        {
          eif_mark_live_pid (proc->pid);
        }
    }
}

processor_t*
processor_get (spid_t pid)
{
  return proc_list[pid].proc;
}

void
processor_unmark (spid_t pid)
{
  // This is a callback from the GC, which will notify us
  // of all unused processors, even those that have already been
  // shutdown, but still have a thread of execution.
  // To avoid double free/shutdown we check first to see if they're
  // still active.
  if (used_pid_set.count (pid) == 1)
    {
      processor_t *proc = processor_get (pid);
      proc->shutdown();
    }
}

void
call_on (spid_t client_pid, spid_t supplier_pid, void* data)
{
  processor_t *client = processor_get (client_pid);
  processor_t *supplier = processor_get (supplier_pid);
  priv_queue_t *pq = client->find_queue_for (supplier);

  if (!supplier->has_backing_thread)
    {
      supplier->spawn();
      supplier->has_backing_thread = true;

      pq->lock();
      pq->log_call (data);
      pq->unlock();
    }
  else
    {
      pq->log_call (data);
    }
}

void
processor_wait_for_all()
{
  processor_t *root_proc = processor_get(0);
  root_proc->has_client = false;
  root_proc->application_loop();

  while(!all_done)
    {
      eif_lock lock (all_done_mutex);
      all_done_cv.wait_for(lock,
                           std::chrono::milliseconds(200),
                           []{return all_done;});
    }
}

void
processor_mark_all (marker_t mark)
{
  for (auto &spid_pair : used_pid_set)
    {
      spid_t pid = spid_pair.first;
      processor *proc = processor_get (pid);
      proc->mark (mark);
    }
}
