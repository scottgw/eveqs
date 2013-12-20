#ifndef _NOTIFY_TOKEN_H
#define _NOTIFY_TOKEN_H

#include "qoq.hpp"
#include <memory>

class processor;

class notify_token {
public:
  notify_token(processor *client);
  notify_token(const notify_token&);

  void next();
  void register_supplier (processor *supplier);
  void wait();
  void notify(processor *client);
  processor *client() const;

private:
  processor *client_;
  std::shared_ptr <mpscq<processor*> > token_queue_;
};

#endif // _NOTIFY_TOKEN_H
