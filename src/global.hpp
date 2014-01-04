#ifndef _GLOBAL_H
#define _GLOBAL_H

#include "processor.hpp"
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_queue.h>

#define MAX_PROCS 1024


// typedef tbb::concurrent_hash_map <spid_t, bool> pid_set_t;

class pid_set : public tbb::concurrent_hash_map <spid_t, bool>
{
public:
  void add (spid_t pid)
  {
    insert (std::pair<spid_t, bool> (pid, true));
  }

  bool has (spid_t pid)
  {
    return count(pid) > 0;
  }
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
  return_pid (spid_t pid);

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
  tbb::concurrent_queue<spid_t> free_pids;

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
