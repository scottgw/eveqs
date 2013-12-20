#include "notify_token.hpp"
#include "processor.hpp"

notify_token::notify_token(processor *client) :
  client_(client),
  token_queue_(std::make_shared <mpscq<processor*> > ())
{
}

notify_token::notify_token(const notify_token& other) :
  client_(other.client_),
  token_queue_(other.token_queue_)
{
}

void
notify_token::register_supplier (processor *supplier)
{ 
  supplier->register_notify_token (*this);
}

processor*
notify_token::client() const
{
  return client_;
}

void
notify_token::wait()
{
  processor* dummy;
  token_queue_->pop (dummy);
}

void
notify_token::notify(processor *client)
{
  token_queue_->push (client);
}
