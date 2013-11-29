#ifndef _GLOBAL_H
#define _GLOBAL_H

#include "processor.hpp"

processor_t
processor_get (spid_t);

void
processor_fresh (void *obj);

void
processor_free_id (processor_t proc);

void
call_on (spid_t client_pid, spid_t supplier_pid, void* data);
#endif // _GLOBAL_H
