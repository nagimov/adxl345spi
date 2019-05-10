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
  this->f = NULL;
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

CountRollupFileADXLWriter::CountRollupFileADXLWriter(const char *filename, bool verbose, int rollupCount)
    : FileADXLWriter(filename, verbose)
{
  this->rollupCount = rollupCount;
  this->count = 0;
  this->name_template = compose_template(filename);
  rollup();
}

CountRollupFileADXLWriter::~CountRollupFileADXLWriter()
{
  delete this->name_template;
  this->name_template = NULL;
}

void CountRollupFileADXLWriter::write(const AccelData& data)
{
  if (this->count > this->rollupCount)
  {
    rollup();
  }
  writeToFile(data);
  this->count++;
}

void CountRollupFileADXLWriter::rollup()
{
  fclose(this->f);
  sprintf(this->filename, this->name_template, get_time());
  this->f = fopen(this->filename, "w");
  this->count = 0;
}

TimeRollupFileADXLWriter::TimeRollupFileADXLWriter(const char *filename, bool verbose, double rollupPeriod)
    : FileADXLWriter(filename, verbose)
{
  this->rollupPeriod = rollupPeriod;
  this->checkpoint = get_time();
  this->name_template = compose_template(filename);
  rollup();
}

TimeRollupFileADXLWriter::~TimeRollupFileADXLWriter()
{
  delete this->name_template;
  this->name_template = NULL;
}

void TimeRollupFileADXLWriter::rollup()
{
  fclose(this->f);
  sprintf(this->filename, this->name_template, get_time());
  this->f = fopen(this->filename, "w");
  this->checkpoint = get_time();
}

void TimeRollupFileADXLWriter::write(const AccelData& data)
{
  if (get_time() - this->checkpoint > this->rollupPeriod)
  {
    rollup();
  }
  writeToFile(data);
}
