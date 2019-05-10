#ifndef ADXL345SPI__PARAMS_H_
#define ADXL345SPI__PARAMS_H_

struct Params {
    bool verbose;        // flag enables console output in mode when we write to file
    bool save;           // flag specified
    char filename[256];  // specified filename to save data
    double freq;         // sampling rate of data stream, Hz
    double samplingTime; // duration of data stream, seconds
};

const struct Params defaults = {
    false,   // silent write to file
    false,   // don't save data to file
    "",
    5,       // 5Hz
    -1,      // infinite duration of data stream
};

int handleCommandLineArgs(int argc, char *argv[], struct Params *p);

#endif //ADXL345SPI__PARAMS_H_
