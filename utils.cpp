#include "utils.h"

unsigned long long getTime()
{
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return (unsigned long long) (tv.tv_sec) * 1000 + (unsigned long long) (tv.tv_usec) / 1000;
}