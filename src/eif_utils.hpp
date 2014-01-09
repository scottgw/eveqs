#ifndef _EIF_UTILS_H
#define _EIF_UTILS_H
#include <mutex>
#include "eif_macros.h"

typedef EIF_REFERENCE marker_t(EIF_REFERENCE *);

void
mark_call_data(marker_t mark, call_data* call);

class eif_block_token
{
public:
  eif_block_token ()
  {
    EIF_ENTER_C;
  }

  ~eif_block_token()
  {
    EIF_EXIT_C;
    RTGC;
  }
};


class eif_lock : eif_block_token, public std::unique_lock <std::mutex>
{
public:
  eif_lock (std::mutex &mutex) :
    eif_block_token (),
    std::unique_lock <std::mutex> (mutex)
  {}
};

#endif // _EIF_UTILS_H
