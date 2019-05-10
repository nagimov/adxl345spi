#ifndef ADXL345SPI__PARAMS_H_
#define ADXL345SPI__PARAMS_H_

struct Params {
    bool verbose;        // flag enables console output in mode when we writeData to file
    bool save;           // flag specified that data save on disk
    bool save_text;    // flag enables text main file format
    char filename[256];  // specified filename to save data
    bool write_fifo;     // flag specified that data save to fifo file
    char fifoname[256];  // specified fifo filename
    double freq;         // sampling rate of data stream, Hz
    double samplingTime; // duration of data stream, seconds
};

const struct Params defaults = {
    false,   // silent writeData to file
    false,   // don't save data to file
    true,   // use text format for main file
    "",
    false,   // don't save data to backup file
    "",
    5,       // 5Hz
    -1,      // infinite duration of data stream
};

int handleCommandLineArgs(int argc, char *argv[], struct Params *p);

#endif //ADXL345SPI__PARAMS_H_
