#ifndef _EIF_LOCK_H
#define _EIF_LOCK_H
#include <mutex>
#include "eif_block_token.hpp"

class eif_lock : eif_block_token, public std::unique_lock <std::mutex>
{
public:
  eif_lock (std::mutex &mutex) :
    eif_block_token (),
    std::unique_lock <std::mutex> (mutex)
  {}
};

#endif // _EIF_LOCK_H
