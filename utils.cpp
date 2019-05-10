#include <sys/time.h>

#include <string>

#include "utils.h"

unsigned long long getTime()
{
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return (unsigned long long) (tv.tv_sec) * 1000 + (unsigned long long) (tv.tv_usec) / 1000;
}

char *compose_template(const char *basename)
{
  char *s = new char[0];
  const char *pos;
  pos = strrchr(basename, '.');
  if (pos == NULL)
  {
    stpcpy(s, basename);
    strcat(s, "_%llu");
  }
  else
  {
    stpncpy(s, basename, pos - basename);
    strcat(s, "_%llu");
    strcat(s, pos);
  }
  return s;
}