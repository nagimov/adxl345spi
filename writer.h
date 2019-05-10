#ifndef ADXL345SPI__WRITER_H_
#define ADXL345SPI__WRITER_H_

#include <string.h>

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
    void write(const AccelData& data);
};

class FileADXLWriter : public ADXLWriter {
  public:
    FileADXLWriter(const char *filename, bool verbose);
    ~FileADXLWriter();
    void write(const AccelData& data);
  protected:
    FILE *f;
    char *filename;
    bool verbose;

    void writeToFile(const AccelData& data);
};

class CountRollupFileADXLWriter : public FileADXLWriter {
  public:
    CountRollupFileADXLWriter(const char *filename, bool verbose, int rollupCount);
    ~CountRollupFileADXLWriter();
    void write(const AccelData& data);
  private:
    void rollup();
    char *name_template;
    int rollupCount;
    int count;
};

class TimeRollupFileADXLWriter : public FileADXLWriter {
  public:
    TimeRollupFileADXLWriter(const char *filename, bool verbose, double rollupPeriod);
    ~TimeRollupFileADXLWriter();
    void write(const AccelData& data);
  private:
    void rollup();
    char *name_template;
    double rollupPeriod;
    unsigned long long checkpoint;
};

ADXLWriter *createWriter(const params& cfg);

#endif //ADXL345SPI__WRITER_H_
