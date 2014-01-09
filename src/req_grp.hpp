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

#ifndef _REQ_GRP_H
#define _REQ_GRP_H
#include <vector>

class processor;
typedef class processor processor_t;

class priv_queue;
typedef class priv_queue priv_queue_t;

class req_grp : public std::vector<priv_queue_t*>
{
public:
  req_grp(processor_t*);

  void add(processor_t*);
  void wait();
  void lock();
  void unlock();
private:
  processor_t *client;
  bool sorted;
};

#endif // _REQ_GRP_H
