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
    void write(const AccelData& data) override;
};

class FileADXLWriter : public ADXLWriter {
  public:
    FileADXLWriter(const char *filename, bool verbose);
    ~FileADXLWriter() override;
    void write(const AccelData& data) override;
  protected:
    FILE *f;
    char *filename;
    bool verbose;

    void writeToFile(const AccelData& data);
};

class RollupFileADXLWriter : public FileADXLWriter {
  public:
    RollupFileADXLWriter(const char *filename, bool verbose);
    ~RollupFileADXLWriter() override;
    void write(const AccelData& data) override;
  protected:
    void rollup();
    virtual bool timeToRollup() = 0;
    virtual void resetRollup() = 0;
    virtual void update() = 0;
  private:
    const char *basename;
    void updateFilename();
};

class TimeRollupFileADXLWriter : public RollupFileADXLWriter {
  public:
    TimeRollupFileADXLWriter(const char *filename, bool verbose, double rollupPeriod);
    ~TimeRollupFileADXLWriter() override;
  protected:
    bool timeToRollup() override;
    void resetRollup() override;
    void update() override
    { };
  private:
    double rollupPeriod;
    unsigned long long checkpoint;
};

class CountRollupFileADXLWriter : public RollupFileADXLWriter {
  public:
    CountRollupFileADXLWriter(const char *filename, bool verbose, int rollupCount);
    ~CountRollupFileADXLWriter() override;
  protected:
    bool timeToRollup() override;
    void resetRollup() override;
    void update() override;
  private:
    int rollupCount;
    int checkpoint;
};

ADXLWriter *createWriter(const params& cfg);

#endif //ADXL345SPI__WRITER_H_
