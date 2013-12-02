#ifndef _EVEQS_H
#define _EVEQS_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
  typedef uint16_t spid_t;


  // 
  // Request chain operations
  //

  // RTS_RC (o) - create request group for o
  void
  eveqs_req_grp_new (spid_t client_pid);

  // RTS_RD (o) - delete chain (release locks?)
  void
  eveqs_req_grp_delete (spid_t client_pid);

  // RTS_RF (o) - wait condition fails

  // RTS_RS (c, s) - add supplier s to current group for c
  void
  eveqs_req_grp_add_supplier (spid_t client_pid, spid_t supplier_pid);

  // RTS_RW (o) - sort all suppliers in the group and get exclusive access
  void
  eveqs_req_grp_lock (spid_t client_pid);

  //
  // Processor creation
  //

  // RTS_PA
  void
  eveqs_processor_fresh (void *);

  //
  // Call logging
  //

  // eif_log_call
  void
  eveqs_call_on (spid_t client_pid, spid_t supplier_pid, void* data);

  int
  eveqs_is_synced_on (spid_t client_pid, spid_t supplier_pid);

  //
  // Callback from garbage collector to indicate that the
  // processor isn't used anymore.
  //
  void
  eveqs_unmarked(spid_t pid);

  void
  eveqs_enumerate_live ();

#ifdef __cplusplus
}
#endif
#endif /* _EVEQS_H */
