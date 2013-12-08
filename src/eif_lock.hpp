#ifndef _EIF_LOCK_H
#define _EIF_LOCK_H
#include <mutex>
#include "eif_block_token.hpp"

class eif_lock
{
public:
  eif_lock (std::mutex &mutex) : unique (mutex)
  {}

private:
  eif_block_token token;
public:
  std::unique_lock<std::mutex> unique;
};

#endif // _EIF_LOCK_H
