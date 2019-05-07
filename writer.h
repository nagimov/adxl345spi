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
  virtual ~ADXLWriter() = 0;
  virtual void write(const AccelData& data) = 0;
};

class ConsoleADXLWriter : public ADXLWriter {
 public:
  void write(const AccelData& data) override;
};

class FileADXLWriter : public ADXLWriter {
 public:
  FileADXLWriter(const char *filename, int verbose);
  ~FileADXLWriter() override;
  void write(const AccelData& data) override;
 private:
  FILE *f;
  const char *filename;
  int verbose{};
};

class RollupFileADXLWriter : public ADXLWriter {
 public:
  void write(const AccelData& data) override;
};

ADXLWriter *createWriter(const params& cfg);

#endif //ADXL345SPI__WRITER_H_
