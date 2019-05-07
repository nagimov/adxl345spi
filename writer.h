#ifndef ADXL345SPI__WRITER_H_
#define ADXL345SPI__WRITER_H_

#include "params.h"

struct AccelData {
    int i;
    int samples;
    unsigned long long time;
    double x;
    double y;
    double z;
};

class ADXLWriter {
 public:
  virtual void write(const AccelData& data) = 0;
};

class ConsoleADXLWriter
    :
        public ADXLWriter {
 public:
  void write(const AccelData& data);
};

class FileADXLWriter
    :
        public ADXLWriter {
 public:
  void write(const AccelData& data);
};

class RollupFileADXLWriter
    :
        public ADXLWriter {
 public:
  void write(const AccelData& data);
};

ADXLWriter *createWriter(const params& cfg);

#endif //ADXL345SPI__WRITER_H_
