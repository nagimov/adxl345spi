#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "params.h"

#define CODE_VERSION  "1.0"
#define MAX_FREQ      3200     // maximal allowed cmdline arg sampling rate of data stream, Hz

void printUsage()
{
  printf("adxl345spi (version %s) \n"
         "Usage: adxl345spi [OPTION]... \n"
         "Read data from ADXL345 accelerometer through SPI interface on Raspberry Pi.\n"
         "\n"
         "Mandatory arguments to long options are mandatory for short options too.\n"
         "  -s, --save FILE     save data to specified FILE (data printed to command-line\n"
         "                      output, if not specified)\n"
         "  -t, --time TIME     set the duration of data stream to TIME seconds\n"
         "                      (default: infinite) [integer]\n"
         "  -f, --freq FREQ     set the sampling rate of data stream to FREQ count per\n"
         "                      second, 1 <= FREQ <= %i (default: %.1f Hz) [integer]\n"
         "  -v, --verbose       enable verbose output when write to file (silent process, \n"
         "                      if not specified)\n"
         "\n"
         "Data is streamed in comma separated format (time, x, y, z), e. g.:\n"
         "   0.0,  10.0,   0.0, -10.0\n"
         "   1.0,   5.0,  -5.0,  10.0\n"
         "   ...,   ...,   ...,   ...\n"
         "  time shows unix timestamp in milliseconds;\n"
         "  x, y and z show acceleration along x, y and z axis in fractions of <g>.\n"
         "\n"
         "Exit status:\n"
         "  0  if OK\n"
         "  1  if error occurred during data reading or wrong cmdline arguments.\n"
         "", CODE_VERSION, MAX_FREQ, defaults.freq);
}

int handleCommandLineArgs(int argc, char *argv[], struct Params *p)
{
  *p = defaults;
  for (int i = 1; i < argc; i++)
  {  // skip argv[0] (program name)
    if ((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--save") == 0))
    {
      p->save = true;
      if (i + 1 <= argc - 1)
      {  // make sure there are enough arguments in argv
        i++;
        strcpy(p->filename, argv[i]);
      }
      else
      {
        printUsage();
        return 1;
      }
    }
    else if ((strcmp(argv[i], "-t") == 0) || (strcmp(argv[i], "--time") == 0))
    {
      if (i + 1 <= argc - 1)
      {
        i++;
        p->samplingTime = atoi(argv[i]);
      }
      else
      {
        printUsage();
        return 1;
      }
    }
    else if ((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--freq") == 0))
    {
      if (i + 1 <= argc - 1)
      {
        i++;
        p->freq = atoi(argv[i]);
        if ((p->freq < 1) || (p->freq > MAX_FREQ))
        {
          printf("Wrong sampling rate specified!\n\n");
          printUsage();
          return 1;
        }
      }
      else
      {
        printUsage();
        return 1;
      }
    }
    else if ((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--verbose") == 0))
    {
      p->verbose = true;
    }
    else
    {
      printUsage();
      return 1;
    }
  }
  return 0;
}