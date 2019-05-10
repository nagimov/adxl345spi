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

class CompositeADXLWriter : public ADXLWriter {
  public:
    CompositeADXLWriter(ADXLWriter **writers, int size);
    ~CompositeADXLWriter();
    void write(const AccelData& data);
  private:
    ADXLWriter **writers;
    int n;
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
    const char *filename;
    bool verbose;
};

class BinaryFileADXLWriter : public ADXLWriter {
  public:
    BinaryFileADXLWriter(const char *filename, bool verbose);
    ~BinaryFileADXLWriter();
    void write(const AccelData& data);
  protected:
    FILE *f;
    const char *filename;
    bool verbose;
};

class FifoFileADXLWriter : public ADXLWriter {
  public:
    FifoFileADXLWriter(const char *filename, bool verbose);
    ~FifoFileADXLWriter();
    void write(const AccelData& data);
  protected:
    FILE *fd;
    const char *filename;
    bool verbose;
};

ADXLWriter *createWriter(const Params& cfg);

#endif //ADXL345SPI__WRITER_H_
