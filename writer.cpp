#include <stdio.h>
#include <string>
#include <assert.h>

#include "writer.h"
#include "utils.h"

ADXLWriter *createWriter(const params& cfg)
{
  if (!cfg.save)
  {
    return new ConsoleADXLWriter();
  }
  else if (cfg.rollupPeriod != -1)
  {
    return new TimeRollupFileADXLWriter(cfg.filename, cfg.verbose, cfg.rollupPeriod);
  }
  else if (cfg.rollupCount != -1)
  {
    return new CountRollupFileADXLWriter(cfg.filename, cfg.verbose, cfg.rollupCount);
  }
  else
  {
    assert(strlen(cfg.filename) != 0 && "filename not initialized");
    return new FileADXLWriter(cfg.filename, cfg.verbose);
  }
}

ADXLWriter::~ADXLWriter() = default;

void ConsoleADXLWriter::write(const AccelData& data)
{
  if (data.samples == -1)
  {
    printf("\r[-/-] %llu : x = %.3f, y = %.3f, z = %.3f", data.time, data.x, data.y, data.z);
  }
  else
  {
    printf("\r[%i/%i] %llu : x = %.3f, y = %.3f, z = %.3f",
           data.i + 1, data.samples, data.time, data.x, data.y, data.z);
  }
  fflush(stdout);
}

FileADXLWriter::FileADXLWriter(const char *filename, bool verbose)
{
  this->f = fopen(filename, "a");
  this->filename = const_cast<char *>(filename);
  this->verbose = verbose;
}

FileADXLWriter::~FileADXLWriter()
{
  fclose(this->f);
  this->filename = nullptr;
}

void FileADXLWriter::writeToFile(const AccelData& data)
{
  if (this->verbose == 1)
  {
    if (data.samples == -1)
    {
      printf("\r[%s] [-/-] %llu : x = %.3f, y = %.3f, z = %.3f", this->filename, data.time, data.x, data.y, data.z);
    }
    else
    {
      printf("\r[%s] [%i/%i] %llu : x = %.3f, y = %.3f, z = %.3f",
             this->filename, data.i + 1, data.samples, data.time, data.x, data.y, data.z);
    }
    fflush(stdout);
  }
  fprintf(f, "%llu,%.5f,%.5f,%.5f\n", data.time, data.x, data.y, data.z);
  fflush(f);
}

void FileADXLWriter::write(const AccelData& data)
{
  writeToFile(data);
}

RollupFileADXLWriter::RollupFileADXLWriter(const char *filename, bool verbose)
    : FileADXLWriter(filename, verbose)
{
  this->basename = filename;
  rollup();
}

void RollupFileADXLWriter::write(const AccelData& data)
{
  printf("ping");
  if (timeToRollup())
  {
    printf("ping2");
    rollup();
  }
  printf("ping3");
  writeToFile(data);
  printf("ping4");
  update();
  printf("ping5");
}

void RollupFileADXLWriter::rollup()
{
  fclose(this->f);
  updateFilename();
  this->f = fopen(this->filename, "a");
  resetRollup();
}

void RollupFileADXLWriter::updateFilename()
{
  sprintf(this->filename, "%s_%llu", this->basename, getTime());
}

TimeRollupFileADXLWriter::TimeRollupFileADXLWriter(const char *filename, bool verbose, double rollupPeriod)
    : RollupFileADXLWriter(filename, verbose)
{
  this->rollupPeriod = rollupPeriod;
  this->checkpoint = getTime();
}

bool TimeRollupFileADXLWriter::timeToRollup()
{
  unsigned long long ct = getTime();
  return ct - this->checkpoint > this->rollupPeriod;
}

void TimeRollupFileADXLWriter::resetRollup()
{
  this->checkpoint = getTime();
}
void TimeRollupFileADXLWriter::write(const AccelData& data)
{
  RollupFileADXLWriter::write(data);
}

CountRollupFileADXLWriter::CountRollupFileADXLWriter(const char *filename, bool verbose, int rollupCount)
    : RollupFileADXLWriter(filename, verbose)
{
  this->rollupCount = rollupCount;
  this->checkpoint = 0;
}

bool CountRollupFileADXLWriter::timeToRollup()
{
  return this->checkpoint >= this->rollupCount;
}

void CountRollupFileADXLWriter::resetRollup()
{
  this->checkpoint = 0;
}

void CountRollupFileADXLWriter::update()
{
  this->checkpoint++;
}
void CountRollupFileADXLWriter::write(const AccelData& data)
{
  RollupFileADXLWriter::write(data);
}
