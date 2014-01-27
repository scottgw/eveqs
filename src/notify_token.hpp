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

#ifndef _NOTIFY_TOKEN_H
#define _NOTIFY_TOKEN_H

#include "qoq.hpp"
#include <memory>

class processor;

class notify_token {
public:
  notify_token(processor *client);
  notify_token(const notify_token&);

  void register_supplier (processor *supplier);
  void wait();
  void notify(processor *client);
  processor *client() const;

private:
  processor *client_;
  std::shared_ptr <mpscq<processor*> > token_queue_;
};

#endif // _NOTIFY_TOKEN_H
