#include <stdio.h>
#include <pigpio.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define CODE_VERSION  "1.0"

#define DATA_FORMAT   0x31  // data format register address
#define DATA_FORMAT_B 0x0B  // data format bytes: +/- 16g range, 13-bit resolution (p. 26 of ADXL345 datasheet)
#define READ_BIT      0x80
#define MULTI_BIT     0x40
#define BW_RATE       0x2C
#define POWER_CTL     0x2D
#define DATAX0        0x32

#define MAX_FREQ      3200     // maximal allowed cmdline arg sampling rate of data stream, Hz

#define SPI_SPEED     2000000  // SPI communication speed, bps

#define COLD_START_SAMPLES 2   // number of samples to be read before outputting data to console (cold start delays)
#define COLD_START_DELAY   0.1 // time delay between cold start reads

struct params {
    int save;            // flag specified
    char filename[256];  // specified filename to save data
    double freq;         // sampling rate of data stream, Hz
    double samplingTime; // duration of data stream, seconds
    double rollupTime;   // rollup time period, seconds
    int rollupCount;     // rollup samples count
};

const struct params defaults = {
    0,   // don't save data to file
    "",
    5,   // 5Hz
    5,   // TODO: -1 = infinite duration of data stream
    -1,  // no rollup by time
    -1   // no rollup by count
};

const double scaleFactor = 2 * 16.0 / 8192.0;  // +/- 16g range, 13-bit resolution

void printUsage() {
    printf("adxl345spi (version %s) \n"
           "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
           "\n"
           "Usage: adxl345spi [OPTION]... \n"
           "Read data from ADXL345 accelerometer through SPI interface on Raspberry Pi.\n"
           "Online help, docs & bug reports: <https://github.com/nagimov/adxl345spi/>\n"
           "\n"
           "Mandatory arguments to long options are mandatory for short options too.\n"
           "  -s, --save FILE     save data to specified FILE (data printed to command-line\n"
           "                      output, if not specified)\n"
           "  -t, --time TIME     set the duration of data stream to TIME seconds\n"
           "                      (default: %.1f seconds) [integer]\n"
           "  -f, --freq FREQ     set the sampling rate of data stream to FREQ samples per\n"
           "                      second, 1 <= FREQ <= %i (default: %.1f Hz) [integer]\n"
           "\n"
           "Data is streamed in comma separated format, e. g.:\n"
           "  time,     x,     y,     z\n"
           "   0.0,  10.0,   0.0, -10.0\n"
           "   1.0,   5.0,  -5.0,  10.0\n"
           "   ...,   ...,   ...,   ...\n"
           "  time shows seconds elapsed since the first reading;\n"
           "  x, y and z show acceleration along x, y and z axis in fractions of <g>.\n"
           "\n"
           "Exit status:\n"
           "  0  if OK\n"
           "  1  if error occurred during data reading or wrong cmdline arguments.\n"
           "", CODE_VERSION, defaults.samplingTime, MAX_FREQ, defaults.freq);
}

int handleCommandLineArgs(int argc, char *argv[], struct params *p) {
    *p = defaults;
    for (int i = 1; i < argc; i++) {  // skip argv[0] (program name)
        if ((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--save") == 0)) {
            p->save = 1;
            if (i + 1 <= argc - 1) {  // make sure there are enough arguments in argv
                i++;
                strcpy(p->filename, argv[i]);
            } else {
                printUsage();
                return 1;
            }
        } else if ((strcmp(argv[i], "-t") == 0) || (strcmp(argv[i], "--time") == 0)) {
            if (i + 1 <= argc - 1) {
                i++;
                p->samplingTime = atoi(argv[i]);
            } else {
                printUsage();
                return 1;
            }
        } else if ((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--freq") == 0)) {
            if (i + 1 <= argc - 1) {
                i++;
                p->freq = atoi(argv[i]);
                if ((p->freq < 1) || (p->freq > MAX_FREQ)) {
                    printf("Wrong sampling rate specified!\n\n");
                    printUsage();
                    return 1;
                }
            } else {
                printUsage();
                return 1;
            }
        } else {
            printUsage();
            return 1;
        }
    }
    return 0;
}

unsigned long long getTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long long) (tv.tv_sec) * 1000 + (unsigned long long) (tv.tv_usec) / 1000;
}

int readBytes(int handle, char *data, int count) {
    data[0] |= READ_BIT;
    if (count > 1) data[0] |= MULTI_BIT;
    return spiXfer(handle, data, data, count);
}

int writeBytes(int handle, char *data, int count) {
    if (count > 1) data[0] |= MULTI_BIT;
    return spiWrite(handle, data, count);
}

int main(int argc, char *argv[]) {
    int i;

    struct params cfg;
    int status = handleCommandLineArgs(argc, argv, &cfg);
    if (status != 0) {
        return status;
    }

    // reading sensor data

    // SPI sensor setup
    int samples = cfg.freq * cfg.samplingTime;
    int success = 1;
    int h, bytes;
    char data[7];
    int16_t x, y, z;
    double tStart, tDuration;
    unsigned long long t;

    if (gpioInitialise() < 0) {
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
    for (i = 0; i < COLD_START_SAMPLES; i++) {
        data[0] = DATAX0;
        bytes = readBytes(h, data, 7);
        if (bytes != 7) {
            success = 0;
        }
        time_sleep(COLD_START_DELAY);
    }

    // real reads happen here
    if (cfg.save == 0) {
        tStart = time_time();
        for (i = 0; i < samples; i++) {
            data[0] = DATAX0;
            bytes = readBytes(h, data, 7);
            if (bytes == 7) {
                x = (data[2] << 8) | data[1];
                y = (data[4] << 8) | data[3];
                z = (data[6] << 8) | data[5];
                t = getTime();
                printf("time = %llu, x = %.3f, y = %.3f, z = %.3f\n",
                       t, x * scaleFactor, y * scaleFactor, z * scaleFactor);
            } else {
                success = 0;
            }
            time_sleep(delay);  // pigpio sleep is accurate enough for console output, not necessary to use nanosleep
        }
        gpioTerminate();
        tDuration = time_time() - tStart;  // need to update current time to give a closer estimate of sampling rate
        printf("%d samples read in %.2f seconds with sampling rate %.1f Hz\n", samples, tDuration, samples / tDuration);
        if (success == 0) {
            printf("Error occurred!");
            return 1;
        }
    } else {
        FILE *f;
        f = fopen(cfg.filename, "w");

        tStart = time_time();
        for (i = 0; i < samples; i++) {
            data[0] = DATAX0;
            bytes = readBytes(h, data, 7);
            if (bytes == 7) {
                x = (data[2] << 8) | data[1];
                y = (data[4] << 8) | data[3];
                z = (data[6] << 8) | data[5];
                t = getTime();
                printf("\r[%s] [%i/%i] %llu : x = %.3f, y = %.3f, z = %.3f",
                       cfg.filename, i + 1, samples, t, x * scaleFactor, y * scaleFactor, z * scaleFactor);
                fflush(stdout);
                fprintf(f, "%llu,%.5f,%.5f,%.5f\n", t, x * scaleFactor, y * scaleFactor, z * scaleFactor);
                fflush(f);
            } else {
                success = 0;
            }
            time_sleep(delay);  // pigpio sleep is accurate enough for console output, not necessary to use nanosleep
        }
        printf("\n");

        gpioTerminate();
        tDuration = time_time() - tStart;  // need to update current time to give a closer estimate of sampling rate

        printf("%d samples read in %.2f seconds with sampling rate %.1f Hz\n", samples, tDuration, samples / tDuration);
        if (success == 0) {
            printf("Error occurred!");
            return 1;
        }
        fclose(f);
    }

    printf("Done\n");
    return 0;
}
