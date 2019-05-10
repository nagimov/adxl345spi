#ifndef ADXL345SPI__PARAMS_H_
#define ADXL345SPI__PARAMS_H_

struct Params {
    bool verbose;        // flag enables console output in mode when we writeData to file
    bool save;           // flag specified that data save on disk
    bool save_binary;    // flag enables text main file format
    char filename[256];  // specified filename to save data
    bool save_backup;    // flag specified that data save to backup file
    bool text_backup;    // flag enables text backup file format
    char backup[256];    // specified backup filename to save data
    double freq;         // sampling rate of data stream, Hz
    double samplingTime; // duration of data stream, seconds
};

const struct Params defaults = {
    false,   // silent writeData to file
    false,   // don't save data to file
    false,   // use text format for main file
    "",
    false,   // don't save data to backup file
    false,   // use binary format for backup file
    "",
    5,       // 5Hz
    -1,      // infinite duration of data stream
};

int handleCommandLineArgs(int argc, char *argv[], struct Params *p);

#endif //ADXL345SPI__PARAMS_H_
