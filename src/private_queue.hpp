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

#ifndef _PRIV_QUEUE_H
#define _PRIV_QUEUE_H
#include "eif_utils.hpp"
#include "spsc.hpp"

class processor;

/* The private queue class.
 *
 * This structure functions as a lock between the client and the supplier.
 * The client is the original client, as this queue may be passed to
 * other clients during lock-passing.
 */
class priv_queue : spsc <call_data*>
{
public:
  /* Construct a private queue.
   * @client is the "rightful" owner of the private queue.
   * @supplier is where all requests logged here will be sent to.
   *
   * Note that although the client will never change over the lifetime of the
   * private queue, all calls will not necessarily originate from the client.
   */
  priv_queue (processor *client, processor *supplier);

  // The original owner of this private queue.
  processor *client;
  // The lifetime end-point of this queue.
  processor *supplier;
  
  /* Locks this private queue.
   *
   * This places this queue in the <supplier>s queue of queues <qoq>.
   * Locking can be recursive, both for the owner and any recipients of
   * lock passing.
   */
  void lock();

  /* Logs a new call to the supplier.
   * @call the call to go to the supplier.
   *
   * This is essentially an enqueue operation on the underlying 
   * concurrent queue, waking the supplier if it was waiting on this
   * queue for more calls.
   */
  void log_call(call_data *call);

  /* Receive a new call.
   * @call will contain the calla fter the return of the function.
   *
   * This will be called by the supplier to receive new calls from the client
   * (**or** some processor that the client has passed its locks to).
   * This is a blocking call if no call data is available.
   */
  void pop_call (call_data *& call)
  {
    pop (call);
  }

  /* Register a wait operation with the <supplier>.
   *
   * The <supplier> will contact the <client> when it has executed some
   * other calls, and thus may have changed a wait-condition.
   */
  void register_wait();

  /* Unlock this queue.
   *
   * Instructs the <supplier> to remove this queue from the <qoq>.
   */
  void unlock();

  /* The locked status of this queue.
   *
   * Reports if this queue is currently locked.
   */
  bool is_locked();

  /* The synchronization status of the queue with the <supplier>.
   *
   * The queue is considered synchronized if the <supplier> is currently processing
   * this queue but is not currently applying any call, and the queue itself
   * is empty.
   */
  bool is_synced();

  // Whether the <supplier> threw an exception.
  bool dirty;

  // GC interaction
public:
  /* Mark the call data.
   * @mark the marking routine to use.
   *
   * This is for integration with the EiffelStudio garbage collector
   * so that the target and arguments of the calls in the call data
   * (which is here outside the view of the runtime) will not be collected.
   */
  void mark (marker_t mark);

private:
  call_data* call_stack_call;
  bool synced;
  int lock_depth;
};


#endif // _PRIV_QUEUE_H
