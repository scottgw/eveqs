#ifndef _SPSC_H
#define _SPSC_H
#include <atomic>
#include <mutex>
#include <condition_variable>

#define __memory_barrier() atomic_thread_fence (std::memory_order_seq_cst)

// // load with 'consume' (data-dependent) memory ordering
template<typename T>
T load_consume(T const* addr)
{
    // hardware fence is implicit on x86
    T v = *const_cast<T const volatile*>(addr);
    __memory_barrier(); // compiler fence
    return v;
}

// store with 'release' memory ordering
template<typename T>
void store_release(T* addr, T v)
{
    // hardware fence is implicit on x86
    __memory_barrier(); // compiler fence
    *const_cast<T volatile*>(addr) = v;
}


// cache line size on modern x86 processors (in bytes)
size_t const cache_line_size = 64;

// single-producer/single-consumer queue
template<typename T>
class spsc_queue_impl
{
public:
  spsc_queue_impl()
  {
    node* n = new node;
    n->next_ = 0;
    tail_ = head_ = first_= tail_copy_ = n;
  }

  ~spsc_queue_impl()
  {
    node* n = first_;
    do
      {
        node* next = n->next_;
        delete n;
        n = next;
      }
    while (n);
  }

  void enqueue(T v)
  {
    node* n = alloc_node();
    n->next_ = 0;
    n->value_ = v;

    store_release(&head_->next_, n);

    head_ = n;
  }

  // returns 'false' if queue is empty
  bool dequeue(T& v)
  {
    if (load_consume(&tail_->next_))
      {
        v = tail_->next_->value_;
        store_release(&tail_, tail_->next_);
        return true;
      }
    else
      {
        return false;
      }
  }

private:
  // internal node structure
  struct node
  {
    node* next_;
    T value_;
  };

  // consumer part
  // accessed mainly by consumer, infrequently be producer
  node* tail_; // tail of the queue

  // delimiter between consumer part and producer part,
  // so that they situated on different cache lines
  char cache_line_pad_ [cache_line_size];

  // producer part
  // accessed only by producer
  node* head_; // head of the queue
  node* first_; // last unused node (tail of node cache)
  node* tail_copy_; // helper (points somewhere between first_ and tail_)

  node* alloc_node()
  {
    // first tries to allocate node from internal node cache,
    // if attempt fails, allocates node via ::operator new()

    if (first_ != tail_copy_)
      {
        node* n = first_;
        first_ = first_->next_;
        return n;
      }
    tail_copy_ = load_consume(&tail_);
    if (first_ != tail_copy_)
      {
        node* n = first_;
        first_ = first_->next_;
        return n;
      }
    node* n = new node;
    return n;
  }

spsc_queue_impl(spsc_queue_impl const&);
spsc_queue_impl & operator = (spsc_queue_impl const&);

};

class spsc
{
public:
  spsc()
  {
  }

  void
  pop(call_data* &data)
  {
    for (int i = 0; i < 128; i++)
      {
        if (q.dequeue(data))
          {
            return;
          }
      }

    {
      eif_lock lock (mutex);
      while (!q.dequeue(data))
        {
          cv.wait(lock);
        }
    }
  }

  void
  push(call_data* data)
  {
    q.enqueue(data);

    {
      std::unique_lock<std::mutex> lock (mutex);
      cv.notify_one();
    }
  }

private:
spsc_queue_impl<call_data*> q;
std::mutex mutex;
std::condition_variable cv;
};

#endif // _SPSC_H
