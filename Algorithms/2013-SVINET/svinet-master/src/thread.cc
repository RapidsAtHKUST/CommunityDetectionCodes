#include "thread.hh"
#include <stdio.h>
#include <string.h>

pthread_mutex_t Thread::_file_mutex;

void
Thread::static_initialize()
{
  pthread_mutex_init(&_file_mutex, NULL);
}

void
Thread::static_uninitialize()
{
  pthread_mutex_destroy(&_file_mutex);
}

Thread::Thread()
{
}

Thread::~Thread()
{
}

int
Thread::create()
{
  pthread_attr_init(&_attr);
  pthread_attr_setdetachstate(&_attr, PTHREAD_CREATE_JOINABLE);
  int r;
  if ((r = pthread_create(&_tid, &_attr, run, (void *)this)) != 0)
    return -1;
  return 0;
}

int
Thread::join()
{
  void *status;
  pthread_attr_destroy(&_attr);
  if (pthread_join(_tid, &status) != 0)
    return -1;
  return 0;
}

void *
Thread::run(void *obj)      // static method
{
  Thread *t = reinterpret_cast<Thread *>(obj);
  t->do_work();
  pthread_exit(NULL);
}
