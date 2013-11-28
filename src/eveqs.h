#ifndef _EVEQS_H
#define _EVEQS_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
  // FIXME: add standard prefix to functions meant to be called
  // more or less directly from a macro in the generated Eiffel code
  typedef uint16_t spid_t;

  // RTS_SRC (o) - create request group for o
  void
  req_grp_new (spid_t client_pid);

  // RTS_RS (c, s) - add supplier s to current group for c
  void
  req_grp_add_supplier (spid_t client_pid, spid_t supplier_pid);

  // RTS_RW (o) - sort all suppliers in the group and get exclusive access
  void
  req_grp_lock (spid_t client_pid);

  // RTS_PA
  void
  processor_fresh (void *);

  // RTS_CC
  void
  intr_call_on (spid_t client_pid, spid_t supplier_pid, void* data);

#ifdef __cplusplus
}
#endif
#endif /* _EVEQS_H */
