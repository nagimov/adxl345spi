#include <stdio.h>
#include <string>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include "writer.h"

ADXLWriter *file_writer(const char *filename, bool use_text, bool verbose)
{
  if (use_text)
  {
    return new FifoFileADXLWriter(filename, verbose);
  }
  else
  {
    return new BinaryFileADXLWriter(filename, verbose);
  }
}

ADXLWriter *createWriter(const Params& cfg)
{
  if (cfg.save && cfg.save_backup)
  {
    assert(strlen(cfg.filename) != 0 && "filename not initialized");
    assert(strlen(cfg.backup) != 0 && "backup filename not initialized");
    ADXLWriter **p = new ADXLWriter *[2];
    p[0] = file_writer(cfg.filename, !cfg.save_binary, cfg.verbose);
    p[1] = file_writer(cfg.backup, cfg.text_backup, false);
    return new CompositeADXLWriter(p, 2);
  }
  else if (cfg.save)
  {
    assert(strlen(cfg.filename) != 0 && "filename not initialized");
    return file_writer(cfg.filename, !cfg.save_binary, cfg.verbose);
  }
  else if (cfg.save_backup)
  {
    assert(strlen(cfg.backup) != 0 && "backup filename not initialized");
    return file_writer(cfg.backup, cfg.text_backup, cfg.verbose);
  }
  else
  {
    return new ConsoleADXLWriter();
  }
}

ADXLWriter::~ADXLWriter() = default;

void ConsoleADXLWriter::writeData(const AccelData& data)
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

CompositeADXLWriter::CompositeADXLWriter(ADXLWriter **writers, int size)
{
  this->writers = writers;
  this->n = size;
}

CompositeADXLWriter::~CompositeADXLWriter()
{
  for (int i = 0; i < this->n; i++)
  {
    delete this->writers[i];
  }
}

void CompositeADXLWriter::writeData(const AccelData& data)
{
  for (int i = 0; i < this->n; i++)
  {
    this->writers[i]->writeData(data);
  }
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

void FileADXLWriter::writeData(const AccelData& data)
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

BinaryFileADXLWriter::BinaryFileADXLWriter(const char *filename, bool verbose)
{
  this->f = fopen(filename, "ab");
  this->filename = filename;
  this->verbose = verbose;
}

BinaryFileADXLWriter::~BinaryFileADXLWriter()
{
  fclose(this->f);
  this->f = NULL;
}

struct Record {
    unsigned long long time;
    double x;
    double y;
    double z;
};

void BinaryFileADXLWriter::writeData(const AccelData& data)
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
  Record r{data.time, data.x, data.y, data.z};
  fwrite((char *) &r, sizeof(Record), 1, this->f);
  fflush(this->f);
}

FifoFileADXLWriter::FifoFileADXLWriter(const char *filename, bool verbose)
{
  mkfifo(filename, 0666);
  this->fd = open(filename, O_WRONLY | O_APPEND | O_NONBLOCK);
  this->filename = filename;
  this->verbose = verbose;
}

FifoFileADXLWriter::~FifoFileADXLWriter()
{
  if (fd > 0)
  {
    close(fd);
    fd = 0;
  }
}

void FifoFileADXLWriter::writeData(const AccelData& data)
{
  if (this->verbose == 1)
  {
    if (data.samples == -1)
    {
      printf("\r[%s] [-/-] %llu : x = %.3f, y = %.3f, z = %.3f \33[K",
             (fd > 0) ? filename : "-", data.time, data.x, data.y, data.z);
    }
    else
    {
      printf("\r[%s] [%i/%i] %llu : x = %.3f, y = %.3f, z = %.3f \33[K",
             (fd > 0) ? filename : "-", data.i + 1, data.samples, data.time, data.x, data.y, data.z);
    }
    fflush(stdout);
  }

  if (fd < 0)
  {
    fd = open(filename, O_WRONLY | O_APPEND | O_NONBLOCK);
    if (fd < 0)
    {
      return;
    }
  }

  char a[1024];
  int n = sprintf(a, "%llu,%.5f,%.5f,%.5f\n", data.time, data.x, data.y, data.z);
  ssize_t num = write(fd, a, n * sizeof(char));
  if (num < 0)
  {
    close(fd);
    fd = -1;
  }
}
