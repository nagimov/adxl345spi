#include <stdio.h>
#include <string>
#include <assert.h>

#include "writer.h"

ADXLWriter *createWriter(const Params& cfg)
{
  if (cfg.save)
  {
    assert(strlen(cfg.filename) != 0 && "filename not initialized");
    return new FileADXLWriter(cfg.filename, cfg.verbose);
  }
  else
  {
    return new ConsoleADXLWriter();
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
  this->filename = filename;
  this->verbose = verbose;
}

FileADXLWriter::~FileADXLWriter()
{
  fclose(this->f);
  this->f = NULL;
}

void FileADXLWriter::write(const AccelData& data)
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