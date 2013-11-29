#include "eveqs.h"
#include "global.hpp"
#include "processor.hpp"

extern "C"
{
  // RTS_RC (o) - create request group for o
  void
  eveqs_req_grp_new (spid_t client_pid)
  {
    processor_t client = processor_get (client_pid);
    client->push_new_req_grp();
  }

  // RTS_RD (o) - delete chain (release locks?)
  void
  eveqs_req_grp_delete (spid_t client_pid)
  {
    processor_t client = processor_get (client_pid);
    client->unlock_req_grp();
  }

  // RTS_RF (o) - wait condition fails


  // RTS_RS (c, s) - add supplier s to current group for c
  void
  eveqs_req_grp_add_supplier (spid_t client_pid, spid_t supplier_pid)
  {
    processor_t client = processor_get (client_pid);
    processor_t supplier = processor_get (supplier_pid);  
    client->add_to_req_grp (supplier);
  }

  // RTS_RW (o) - sort all suppliers in the group and get exclusive access
  void
  eveqs_req_grp_lock (spid_t client_pid)
  {
    processor_t client = processor_get (client_pid);
    client->lock_req_grp();
  }


  //
  // Processor creation
  //

  // RTS_PA
  void
  eveqs_processor_fresh (void *obj)
  {
    processor_fresh (obj);
  }

  //
  // Call logging
  //

  // eif_log_call
  void
  eveqs_call_on (spid_t client_pid, spid_t supplier_pid, void* data)
  {
    // FIXME: do as before and get both the client and supplier and turn into
    // a proper call on the client?
    call_on (client_pid, supplier_pid, data);
  }
}
