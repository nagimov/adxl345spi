#include <stdio.h>
#include <pigpio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define DATA_FORMAT   0x31  // data format register address
#define DATA_FORMAT_B 0x0B  // data format bytes: +/- 16g range, 13-bit resolution (p. 26 of ADXL345 datasheet)
#define READ_BIT      0x80
#define MULTI_BIT     0x40
#define BW_RATE       0x2C
#define POWER_CTL     0x2D
#define DATAX0        0x32

const char codeVersion[3] = "0.2";  // code version number
const int timeDefault = 5;  // default duration of data stream, seconds
const int freqDefault = 5;  // default sampling rate of data stream, Hz
const int freqMax = 3200;  // maximal allowed cmdline arg sampling rate of data stream, Hz
const int speedSPI = 2000000;  // SPI communication speed, bps
const int freqMaxSPI = 100000;  // maximal possible communication sampling rate through SPI, Hz (assumption)
const int coldStartSamples = 2;  // number of samples to be read before outputting data to console (cold start delays)
const double coldStartDelay = 0.1;  // time delay between cold start reads
const double accConversion = 2 * 16.0 / 8192.0;  // +/- 16g range, 13-bit resolution
const double tStatusReport = 1;  // time period of status report if data read to file, seconds

void printUsage() {
    printf( "adxl345spi (version %s) \n"
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
            "                      (default: %i seconds) [integer]\n"
            "  -f, --freq FREQ     set the sampling rate of data stream to FREQ samples per\n"
            "                      second, 1 <= FREQ <= %i (default: %i Hz) [integer]\n"
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
            "", codeVersion, timeDefault, freqMax, freqDefault);
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

    // handling command-line arguments

    int bSave = 0;
    char vSave[256] = "";
    double vTime = timeDefault;
    double vFreq = freqDefault;
    for (i = 1; i < argc; i++) {  // skip argv[0] (program name)
        if ((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--save") == 0)) {
            bSave = 1;
            if (i + 1 <= argc - 1) {  // make sure there are enough arguments in argv
                i++;
                strcpy(vSave, argv[i]);
            }
            else {
                printUsage();
                return 1;
            }
        }
        else if ((strcmp(argv[i], "-t") == 0) || (strcmp(argv[i], "--time") == 0)) {
            if (i + 1 <= argc - 1) {
                i++;
                vTime = atoi(argv[i]);
            }
            else {
                printUsage();
                return 1;
            }
        }
        else if ((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--freq") == 0)) {
            if (i + 1 <= argc - 1) {
                i++;
                vFreq = atoi(argv[i]);
                if ((vFreq < 1) || (vFreq > 3200)) {
                    printf("Wrong sampling rate specified!\n\n");
                    printUsage();
                    return 1;
                }
            }
            else {
                printUsage();
                return 1;
            }
        }
        else {
            printUsage();
            return 1;
        }
    }

    // reading sensor data

    // SPI sensor setup
    int samples = vFreq * vTime;
    int samplesMaxSPI = freqMaxSPI * vTime;
    int success = 1;
    int h, bytes;
    char data[7];
    int16_t x, y, z;
    double tStart, tDuration, t;
    if (gpioInitialise() < 0) {
        printf("Failed to initialize GPIO!");
        return 1;
    }
    h = spiOpen(0, speedSPI, 3);
    data[0] = BW_RATE;
    data[1] = 0x0F;
    writeBytes(h, data, 2);
    data[0] = DATA_FORMAT;
    data[1] = DATA_FORMAT_B;
    writeBytes(h, data, 2);
    data[0] = POWER_CTL;
    data[1] = 0x08;
    writeBytes(h, data, 2);

    double delay = 1.0 / vFreq;  // delay between reads in seconds

    // depending from the output mode (print to cmdline / save to file) data is read in different ways

    // for cmdline output, data is read directly to the screen with a sampling rate which is *approximately* equal...
    // but always less than the specified value, since reading takes some time

    if (bSave == 0) {
        // fake reads to eliminate cold start timing issues (~0.01 s shift of sampling time after the first reading)
        for (i = 0; i < coldStartSamples; i++) {
            data[0] = DATAX0;
            bytes = readBytes(h, data, 7);
            if (bytes != 7) {
                success = 0;
            }
            time_sleep(coldStartDelay);
        }
        // real reads happen here
        tStart = time_time();
        for (i = 0; i < samples; i++) {
            data[0] = DATAX0;
            bytes = readBytes(h, data, 7);
            if (bytes == 7) {
                x = (data[2]<<8)|data[1];
                y = (data[4]<<8)|data[3];
                z = (data[6]<<8)|data[5];
                t = time_time() - tStart;
                printf("time = %.3f, x = %.3f, y = %.3f, z = %.3f\n",
                       t, x * accConversion, y * accConversion, z * accConversion);
                }
            else {
                success = 0;
            }
            time_sleep(delay);  // pigpio sleep is accurate enough for console output, not necessary to use nanosleep
        }
        gpioTerminate();
        tDuration = time_time() - tStart;  // need to update current time to give a closer estimate of sampling rate
        printf("%d samples read in %.2f seconds with sampling rate %.1f Hz\n", samples, tDuration, samples/tDuration);
        if (success == 0) {
            printf("Error occurred!");
            return 1;
        }
    }

    // for the file output, data is read with a maximal possible sampling rate (around 30,000 Hz)...
    // and then accurately rescaled to *exactly* match the specified sampling rate...
    // therefore, saved data can be easily analyzed (e. g. with fft)
    else {
        // reserve vectors for file-output arrays: time, x, y, z
        // arrays will not change their lengths, so separate track of the size is not needed
        double *at, *ax, *ay, *az;
        at = malloc(samples * sizeof(double));
        ax = malloc(samples * sizeof(double));
        ay = malloc(samples * sizeof(double));
        az = malloc(samples * sizeof(double));

        // reserve vectors for raw data: time, x, y, z
        // maximal achievable sampling rate depends from the hardware...
        // in my case, for Raspberry Pi 3 at 2 Mbps SPI bus speed sampling rate never exceeded ~30,000 Hz...
        // so to be sure that there is always enough memory allocated, freqMaxSPI is set to 60,000 Hz
        double *rt, *rx, *ry, *rz;
        rt = malloc(samplesMaxSPI * sizeof(double));
        rx = malloc(samplesMaxSPI * sizeof(double));
        ry = malloc(samplesMaxSPI * sizeof(double));
        rz = malloc(samplesMaxSPI * sizeof(double));

        printf("Reading %d samples in %.1f seconds with sampling rate %.1f Hz...\n", samples, vTime, vFreq);
        int statusReportedTimes = 0;
        double tCurrent, tClosest, tError, tErrorPrev, tLeft;
        int j, jClosest;

        tStart = time_time();
        int samplesRead;
        for (i = 0; i < samplesMaxSPI; i++) {
            data[0] = DATAX0;
            bytes = readBytes(h, data, 7);
            if (bytes == 7) {
                x = (data[2]<<8)|data[1];
                y = (data[4]<<8)|data[3];
                z = (data[6]<<8)|data[5];
                t = time_time();
                rx[i] = x * accConversion;
                ry[i] = y * accConversion;
                rz[i] = z * accConversion;
                rt[i] = t - tStart;
            }
            else {
                gpioTerminate();
                printf("Error occurred!");
                return 1;
            }
            tDuration = t - tStart;
            if (tDuration > tStatusReport * ((float)statusReportedTimes + 1.0)) {
                statusReportedTimes++;
                tLeft = vTime - tDuration;
                if (tLeft < 0) {
                    tLeft = 0.0;
                }
                printf("%.2f seconds left...\n", tLeft);
            }
            if (tDuration > vTime) {  // enough data read
                samplesRead = i;
                break;
            }
        }
        gpioTerminate();
        printf("Writing to the output file...\n");
        for (i = 0; i < samples; i++) {
            if (i == 0) {  // always get the first reading from position 0
                tCurrent = 0.0;
                jClosest = 0;
                tClosest = rt[jClosest];
            }
            else {
                tCurrent = (float)i * delay;
                tError = fabs(tClosest - tCurrent);
                tErrorPrev = tError;
                for (j = jClosest; j < samplesRead; j++) {  // lookup starting from previous j value
                    if (fabs(rt[j] - tCurrent) < tError) {  // in order to save some iterations
                        jClosest = j;
                        tClosest = rt[jClosest];
                        tErrorPrev = tError;
                        tError = fabs(tClosest - tCurrent);
                    }
                    else {
	                    if (tError > tErrorPrev) {  // if the error starts growing
    	                	break;                  // break the loop since the minimal error point passed
	                    }
                    }
                }  // when this loop is ended, jClosest and tClosest keep coordinates of the closest (j, t) point
            }
            ax[i] = rx[jClosest];
            ay[i] = ry[jClosest];
            az[i] = rz[jClosest];
            at[i] = tCurrent;
        }
        FILE *f;
        f = fopen(vSave, "w");
        fprintf(f, "time, x, y, z\n");
        for (i = 0; i < samples; i++) {
            fprintf(f, "%.5f, %.5f, %.5f, %.5f \n", at[i], ax[i], ay[i], az[i]);
        }
        fclose(f);
    }

    printf("Done\n");
    return 0;
}
