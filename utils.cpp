#include <sys/time.h>
#include <string.h>

#include "utils.h"

unsigned long long get_time()
{
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return (unsigned long long) (tv.tv_sec) * 1000 + (unsigned long long) (tv.tv_usec) / 1000;
}