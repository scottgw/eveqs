#include <ck_fifo.h>
#include <stdlib.h>
#include "spsc_queue.h"

typedef ck_fifo_spsc_t spsc_queue_t;

spsc_queue_t*
spsc_queue()
{
  ck_fifo_spsc_entry_t *stub = malloc (sizeof(*stub));
  spsc_queue_t *q = malloc (sizeof(*q));
  ck_fifo_spsc_init (q, stub);

  return q;
}

void spsc_enqueue(spsc_queue_t *q, void *data)
{
  ck_fifo_spsc_entry_t *node = malloc (sizeof(*node));
  ck_fifo_spsc_enqueue (q, node, data);
}

bool spsc_dequeue(spsc_queue_t *q, void *data)
{
  ck_fifo_spsc_entry_t *old_node;
  bool success = ck_fifo_spsc_dequeue (q, data);
  old_node = ck_fifo_spsc_recycle (q);
  if (old_node)
    {
      free (old_node);
    }

  return success;
}

bool
spsc_is_empty(spsc_queue_t *q)
{
  return ck_fifo_spsc_isempty(q);
}
