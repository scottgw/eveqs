#ifndef _EIF_BLOCK_TOKEN_H
#define _EIF_BLOCK_TOKEN_H
#include "eif_macros.h"

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

#endif // _EIF_BLOCK_TOKEN_H
