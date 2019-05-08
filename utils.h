#ifndef _UTILS_H_
#define _UTILS_H_

#include <sys/time.h>

unsigned long long getTime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (unsigned long long) (tv.tv_sec) * 1000 + (unsigned long long) (tv.tv_usec) / 1000;
}

#endif //_UTILS_H_
