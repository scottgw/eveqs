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
typedef class processor processor_t;

class priv_queue : spsc <call_data*>
{
public:
  priv_queue (processor_t*, processor_t*);
  processor_t* client;
  processor_t* supplier;
  
  void lock();
  void log_call(call_data*);
  void pop_call (call_data *&);
  void register_wait();
  void unlock();

  bool is_synced();

  bool dirty;

  // GC interaction
public:
  void mark (marker_t mark);

private:
  bool synced;
  int lock_depth;
};


#endif // _PRIV_QUEUE_H
