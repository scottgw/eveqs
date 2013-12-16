#include <assert.h>
#include "eif_utils.hpp"

void
mark_call_data(marker_t mark, call_data* call)
{
  assert (call && "Cannot mark NULL calls");
  EIF_REFERENCE *targ = (EIF_REFERENCE*)(&call->target);
  *targ = mark (targ);

  for (size_t i = 0; i < call->count; i++)
    {
      if (call->argument[i].type == SK_REF)
        {
          EIF_REFERENCE *arg = &call->argument[i].it_r;
          *arg = mark (arg);
        }
    }
}
