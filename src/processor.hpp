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
#include <vector>
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
#include "queue_cache.hpp"

/* A specialized version of <spsc> that is for notification.
 *
 * The use of <spsc> allows notifications to not be lost, although
 * of course it can only be used between a single receiver and sender
 * at one time.
 */
struct notifier : spsc <void*> {

  void wait (void *expected)
  {
    void *result;
    this->pop(result, 64);
    if (result != expected)
      {
	printf ("Unexpected result!\n");
      }
  }

  void wake (void *finished = NULL)
  {
    this->push(finished);
  }
};

/* The SCOOP logical unit of processing.
 *
 * The processor is responsible for receiving calls, executing them, and
 * notifying clients of changes.
 */
class processor
{
public:
  /* Construct a new processor.
   * @_pid the processor ID that will be used to identify this processor
   *       to the Eiffel runtime
   * @_has_backing_thread whether the processor alreadyd has a backing thread,
   *                      normally this is false, but for the root processor
   *                      it will already be true.
   *
   * The new processor will usually not yet have a thread backing it.
   */
  processor(spid_t _pid,
            bool _has_backing_thread = false);

  /* The main loop of the processor.
   *
   * Normally this will be called when the thread spawns but here we expose it
   * so that it may be called externally for the root thread (whose thread is
   * the main thread of the program, and thus already exists).
   */
  void application_loop();

  /* Send a shutdown message.
   *
   * Sending a shutdown message will cause the thread to shutdown when it
   * receives it. It will only receive it after it has processed the other
   * requests in its <qoq>, so it may not take effect immediately.
   */
  void shutdown();

private:
  std::vector<processor*> subordinates;

public:
  /* The cache of private queues.
   */
  queue_cache cache;

public:
  /* The queue of queues.
   * 
   * This will be used by private queues to add new calls for this processor
   * to apply.
   */
  mpscq <priv_queue*> qoq;

public:
  /* A stack of <req_grp>s.
   * 
   * The vector here is used as a stack, which mirrors the processors locked
   * in the real call stack.
   */
  std::vector <req_grp> group_stack;

public:
  /* Register another processors <notify_token>.
   * @token the token to add to this processor.
   *
   * These list of tokens will be notified in some way when this the heap
   * protected by this processor may have changed. This is used to implement
   * notification for things like wait conditions.
   */
  void register_notify_token(notify_token token);

  /* Notify token for wait conditions.
   * 
   * This will be used as an argument to other <processor>'s
   * <register_notify_token>.
   */
  notify_token my_token;

private:
  std::queue <notify_token> token_queue;
  void notify_next(processor *);


public:

  /* Has a client.
   * 
   * This is used to prevent an active
   * processor, which may not have any references to it, from being collected.
   */
  bool has_client;

  /* The currently executing call.
   * 
   * This will be traced during marking.
   */
  call_data* executing_call;

  /* The marking routine to trace this processor's <priv_queue>s.
   */
  void mark (marker_t mark);

public:
  /* A result notifier.
   * 
   * This is the notifier that this processor will wait on when
   * it asks another processor for a result.
   */
  notifier result_notify;

public:
  /* Ask the Eiffel runtime to make a new thread for this processor.
   */
  void spawn();
  
  /* Has an associated thread.
   * 
   * True if this processor has a thread spawned to back it.
   */
  volatile bool has_backing_thread;

  /* Notifier for startup.
   *
   * Lets the constructing thread know that the backing
   * thread for this processor has been spawned.
   * It is important for GC that the thread that requested this processor
   * to spawn a thread doesn't proceed until it has been constructed.
   */
  notifier startup_notify;

  /* The processor ID.
   */
  spid_t pid;

private:
  /* A vacuous pointer object to satisfy the Eiffel runtime's requirement
   * for an object to be the "current" object for a thread.
   */
  std::shared_ptr<std::nullptr_t> parent_obj;

  /* Try to execute a call from the private queue.
   * @pq private queue to take the call from
   * @call the call to apply
   */
  void try_call (priv_queue *pq, call_data *call);

  void process_priv_queue(priv_queue*);

};


#endif // _PROCESSOR_H
