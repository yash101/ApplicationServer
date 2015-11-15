/*
 * AutoMutex.h
 *
 *  Created on: Nov 8, 2015
 *      Author: yash
 */

#ifndef AUTOMUTEX_H_
#define AUTOMUTEX_H_

#include <stddef.h>
#include <mutex>

namespace tool
{
  template<typename MutexClassType>
  class AutoMutex
  {
  public:
    MutexClassType* mutex;

    AutoMutex() :
      mutex(NULL)
    {}

    AutoMutex(MutexClassType& mtx) :
      mutex(&mtx)
    {
      if(mutex != NULL)
        mutex->lock();
    }

    AutoMutex(MutexClassType* mtx) :
      mutex(mtx)
    {
      if(mutex != NULL)
      {
        mutex->lock();
      }
    }

    virtual ~AutoMutex()
    {
      if(mutex != NULL)
      {
        mutex->unlock();
      }
    }
  };

  typedef AutoMutex<std::mutex> AutoSTDMutex;
}

#endif /* AUTOMUTEX_H_ */
