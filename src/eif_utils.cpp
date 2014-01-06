#include <assert.h>
#include "eif_utils.hpp"

static
void
mark_ref (marker_t mark, EIF_REFERENCE *ref)
{
  assert (*ref);
  *ref = mark (ref);
}

void
mark_call_data(marker_t mark, call_data* call)
{
  assert (call && "Cannot mark NULL calls");
  mark_ref (mark, (EIF_REFERENCE*)(&call->target));

  for (size_t i = 0; i < call->count; i++)
    {
      if (call->argument[i].type == SK_REF)
        {
	  mark_ref (mark, &call->argument[i].it_r);
        }
    }
}

