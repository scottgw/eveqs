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

#ifndef _PROCESSOR_H
#define _PROCESSOR_H
#include <memory>
#include <stack>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <unordered_map>
#include "qoq.hpp"
#include "eif_utils.hpp"
#include "private_queue.hpp"
#include "req_grp.hpp"
#include "spsc.hpp"
#include "eveqs.h"
#include "notify_token.hpp"

typedef tbb::concurrent_bounded_queue <void*> notifier_queue;

struct notifier : notifier_queue {
  void wait ()
  {
    void* dummy;
    eif_pop(static_cast<notifier_queue*> (this), dummy);
  }

  void wake ()
  {
    void* dummy = NULL;
    eif_push(static_cast<notifier_queue*> (this), dummy);
  }
};

class processor
{
public:
  processor(spid_t _pid,
            bool _has_backing_thread = false);

  // lifetime operations
  void application_loop();
  void shutdown();

  // queue cache
public:
  priv_queue_t* find_queue_for(processor_t*);

  // queue operations, move to private and offer clients
  // only pushing capabilities?
public:
  mpscq <priv_queue*> qoq;

  // separate argument stacks
public:
  std::stack <req_grp> group_stack;

  // registration for wait condition notification
public:
  void register_notify_token(notify_token token);
  notify_token my_token;

private:
  std::queue <notify_token> token_queue;
  void notify_next(processor *);

  // GC interaction
public:
  bool has_client;   // activity flag for the GC
  call_data* executing_call;
  void mark (marker_t mark);

  // result notification
public:
  notifier result_notify;

  // Interoperability with the ES runtime
public:
  void spawn();
  bool has_backing_thread;
  spid_t pid;
  std::shared_ptr<nullptr_t> parent_obj;
  eif_global_context_t *globals;
  void try_call (priv_queue_t*, call_data*);

  // private stuff, no particular grouping
private:
  void process_priv_queue(priv_queue_t*);
  std::unordered_map <processor_t*, priv_queue_t*> queue_cache;
};


#endif // _PROCESSOR_H
