#ifndef TSQUEUE_HH
#define TSQUEUE_HH

#include <queue>
#include <deque>
#include <iostream>

template<class T>
class TSQueue : public std::queue<T *>
{
  typedef std::queue<T *> Queue;
  
public:
  TSQueue() { }
  ~TSQueue() { }
  
  void push(T *);
  T *pop();
  T * top();
  void clear();
  
private:
  mutable CondMutex _cm;
};

template<class T> inline void 
TSQueue<T>::push(T *el)
{
  _cm.lock();
  Queue::push(el);
  _cm.signal();
  _cm.unlock();
}

template<class T> inline T *
TSQueue<T>::pop()
{
  T *result;
  for (;;) {
    _cm.lock();
    while (Queue::empty())
      _cm.wait();
    if (!Queue::empty()) {
      result = Queue::front();
      Queue::pop();
      _cm.unlock();
      return result;
    } else
      _cm.unlock();
  }
  return NULL;
}

template<class T> inline void
TSQueue<T>::clear()
{
  _cm.lock();
  while (!Queue::empty())
    Queue::pop();
  _cm.unlock();
}

#endif
