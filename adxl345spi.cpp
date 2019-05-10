#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pigpio.h>
#include <time.h>

#include "params.h"
#include "writer.h"
#include "utils.h"

#define DATA_FORMAT   0x31  // data format register address
#define DATA_FORMAT_B 0x0B  // data format bytes: +/- 16g range, 13-bit resolution (p. 26 of ADXL345 datasheet)
#define READ_BIT      0x80
#define MULTI_BIT     0x40
#define BW_RATE       0x2C
#define POWER_CTL     0x2D
#define DATAX0        0x32

#define SPI_SPEED     2000000  // SPI communication speed, bps

#define COLD_START_SAMPLES 2   // number of samples to be read before outputting data to console (cold start delays)
#define COLD_START_DELAY   0.1 // time delay between cold start reads

const double scaleFactor = 2 * 16.0 / 8192.0;  // +/- 16g range, 13-bit resolution

int readBytes(int handle, char *data, int count)
{
  data[0] |= READ_BIT;
  if (count > 1) data[0] |= MULTI_BIT;
  return spiXfer(handle, data, data, count);
}

int writeBytes(int handle, char *data, int count)
{
  if (count > 1) data[0] |= MULTI_BIT;
  return spiWrite(handle, data, count);
}

int main(int argc, char *argv[])
{
  int i;

  struct params cfg;
  int status = handleCommandLineArgs(argc, argv, &cfg);
  if (status != 0)
  {
    return status;
  }

  ADXLWriter *writer = createWriter(cfg);

  // reading sensor data

  // SPI sensor setup
  int samples = cfg.samplingTime == -1 ? -1 : cfg.freq * cfg.samplingTime;
  int success = 1;
  int h, bytes;
  char data[7];
  int16_t x, y, z;
  double tStart, tDuration;
  unsigned long long t;

  if (gpioInitialise() < 0)
  {
    printf("Failed to initialize GPIO!");
    return 1;
  }

  h = spiOpen(0, SPI_SPEED, 3);
  data[0] = BW_RATE;
  data[1] = 0x0F;
  writeBytes(h, data, 2);
  data[0] = DATA_FORMAT;
  data[1] = DATA_FORMAT_B;
  writeBytes(h, data, 2);
  data[0] = POWER_CTL;
  data[1] = 0x08;
  writeBytes(h, data, 2);

  double delay = 1.0 / cfg.freq;  // delay between reads in seconds

  // fake reads to eliminate cold start timing issues (~0.01 s shift of sampling time after the first reading)
  for (i = 0; i < COLD_START_SAMPLES; i++)
  {
    data[0] = DATAX0;
    bytes = readBytes(h, data, 7);
    if (bytes != 7)
    {
      success = 0;
    }
    time_sleep(COLD_START_DELAY);
  }

  // real reads happen here
  if (cfg.samplingTime != -1)
  {
    tStart = time_time();
    for (i = 0; i < samples; i++)
    {
      data[0] = DATAX0;
      bytes = readBytes(h, data, 7);
      if (bytes == 7)
      {
        x = (data[2] << 8) | data[1];
        y = (data[4] << 8) | data[3];
        z = (data[6] << 8) | data[5];
        t = getTime();
        writer->write(AccelData{i, samples, t, x * scaleFactor, y * scaleFactor, z * scaleFactor});
      }
      else
      {
        success = 0;
      }
      time_sleep(delay);  // pigpio sleep is accurate enough for console output, not necessary to use nanosleep
    }
    printf("\n");
    gpioTerminate();
    tDuration = time_time() - tStart;  // need to update current time to give a closer estimate of sampling rate
    printf("%d count read in %.2f seconds with sampling rate %.1f Hz\n",
           samples,
           tDuration,
           samples / tDuration);
    if (success == 0)
    {
      printf("Error occurred!");
      return 1;
    }
  }
  else
  {
    while (1)
    {
      data[0] = DATAX0;
      bytes = readBytes(h, data, 7);
      if (bytes == 7)
      {
        x = (data[2] << 8) | data[1];
        y = (data[4] << 8) | data[3];
        z = (data[6] << 8) | data[5];
        t = getTime();
        writer->write(AccelData{i, samples, t, x * scaleFactor, y * scaleFactor, z * scaleFactor});
      }
      time_sleep(delay);  // pigpio sleep is accurate enough for console output, not necessary to use nanosleep
    }
  }

  delete writer;
  printf("Done\n");
  return 0;
}
