#ifndef THREAD_HH
#define THREAD_HH

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>

class Thread {
public:
  Thread();
  virtual ~Thread();
  
  int create();
  int join();
  pthread_t id() const { return _tid; }

  virtual int do_work() { return 0; }
  
  static void static_initialize();
  static void static_uninitialize();
  
protected:
  static void *run(void *);
  
  bool _done;
  pthread_t _tid;
  pthread_attr_t _attr;
  static pthread_mutex_t _file_mutex;
};

class Mutex {
public:
  Mutex();
  ~Mutex();
  
  int lock();
  int unlock();
  int try_lock();

private:
  pthread_mutex_t _mutex;
  pthread_t _owner;
  
  Mutex &operator=(const Mutex &);
  Mutex(const Mutex &);

  friend class CondMutex;
};

inline
Mutex::Mutex()
{
  pthread_mutex_init(&_mutex, 0);
}

inline
Mutex::~Mutex()
{
  pthread_mutex_destroy(&_mutex);
}

inline int
Mutex::lock()
{
  int r = pthread_mutex_lock(&_mutex);
  return r;
}

inline int
Mutex::try_lock()
{
  int r = pthread_mutex_trylock(&_mutex);
  return r;
}

inline int
Mutex::unlock()
{
  return pthread_mutex_unlock(&_mutex);
}

class CondMutex {
public:
  CondMutex();
  ~CondMutex();

  int wait();
  int signal();
  int broadcast();
  int lock() { return _mutex.lock(); }
  int unlock() { return _mutex.unlock(); }

private:
  pthread_cond_t _cond;
  Mutex _mutex;
  
  CondMutex &operator=(const CondMutex &);
  CondMutex(const CondMutex &);
};

inline CondMutex::CondMutex()
{
  pthread_cond_init(&_cond, NULL);
}

inline CondMutex::~CondMutex()
{
  pthread_cond_destroy(&_cond);
}

inline int
CondMutex::wait()
{
  return pthread_cond_wait(&_cond, &_mutex._mutex);
}

inline int
CondMutex::signal()
{
  return pthread_cond_signal(&_cond);
}

inline int
CondMutex::broadcast()
{
  return pthread_cond_broadcast(&_cond);
}
#endif
