#ifndef ADXL345SPI__PARAMS_H_
#define ADXL345SPI__PARAMS_H_

struct params {
    int verbose;         // flag enables console output in mode when we write to file
    int save;            // flag specified
    char filename[256];  // specified filename to save data
    double freq;         // sampling rate of data stream, Hz
    double samplingTime; // duration of data stream, seconds
    double rollupTime;   // rollup time period, seconds
    int rollupCount;     // rollup samples count
};

const struct params defaults = {
    0,   // silent write to file
    0,   // don't save data to file
    "",
    5,   // 5Hz
    -1,  // infinite duration of data stream
    -1,  // no rollup by time
    -1   // no rollup by count
};

#endif //ADXL345SPI__PARAMS_H_
