#ifndef _SPSC_QUEUE_H
#define _SPSC_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

  struct ck_fifo_spsc;
  typedef struct ck_fifo_spsc spsc_queue_t;

  spsc_queue_t*
  spsc_queue();

  void
  spsc_enqueue(spsc_queue_t *q, void *data);

  bool
  spsc_dequeue(spsc_queue_t *q, void *data);

  bool
  spsc_is_empty(spsc_queue_t *q);


#ifdef __cplusplus
}
#endif

#endif // _SPSC_QUEUE_H
