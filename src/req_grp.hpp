#ifndef _REQ_GRP_H
#define _REQ_GRP_H
#include <vector>

class processor;
typedef class processor *processor_t;

class priv_queue;
typedef class priv_queue *priv_queue_t;

class req_grp : public std::vector<priv_queue_t>
{
public:
  req_grp();

  void add(processor_t, processor_t);
  void lock();
  void unlock();
private:
  bool sorted;
};

#endif // _REQ_GRP_H
