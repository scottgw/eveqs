#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <atomic>
#include "mpmc.hpp"
#include "processor.hpp"

#define MAX_PROCS 1024

class pid_set
{
public:
  pid_set()
  {
    size_ = 0;
    for (int i = 0; i < MAX_PROCS; i++)
      {
	proc_set [i] = false;
      }
  }

  void add (spid_t pid)
  {
    bool expected = false;
    if (proc_set [pid].compare_exchange_strong (expected, true))
      {
	size_++;
      }
  }

  bool has (spid_t pid)
  {
    return proc_set [pid];
  }

  bool erase (spid_t pid)
  {
    bool result = proc_set [pid].exchange (false);
    if (result)
      {
	size_--;
      }
    return result;
  }

  size_t size() const 
  {
    return size_;
  }

private:
  std::atomic<size_t> size_;
  std::atomic<bool> proc_set [MAX_PROCS];
};


class processor_registry
{
public:
  processor_registry ();

  processor*
  create_fresh (void* obj);

  processor*
  operator[] (spid_t pid);

  void
  return_processor (processor* proc);

  // GC activities
public:
  void
  enumerate_live ();

  void
  mark_all (marker_t mark);

  void
  unmark (spid_t pid);

  void
  wait_for_all();

private:
  processor* procs[MAX_PROCS];
  pid_set used_pids;
  mpmc_bounded_queue<spid_t> free_pids;

  // GC
private:
  std::atomic<bool> is_marking;

  // end of life notification
private:
  bool all_done;
  std::mutex all_done_mutex;
  std::condition_variable all_done_cv;
};

void
call_on (spid_t client_pid, spid_t supplier_pid, void* data);


extern processor_registry registry;



#endif // _GLOBAL_H
