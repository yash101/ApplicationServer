#ifndef AUTO_MUTEX_H
#define AUTO_MUTEX_H
#include <stddef.h>

template<typename Mutex_T> class auto_mutex
{
private:
  Mutex_T* mutex;
public:
  auto_mutex() :
    mutex(NULL)
  {}

  auto_mutex(Mutex_T* mtx) :
    mutex(mtx)
  {
    mutex->lock();
  }

  ~auto_mutex()
  {
    if(mutex != NULL) mutex->unlock();
  }
};

#endif // AUTO_MUTEX_H
