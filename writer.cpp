#include <stdio.h>
#include "writer.h"

ADXLWriter *createWriter(const params& cfg)
{
  if (cfg.save == 0)
  {
    return new ConsoleADXLWriter();
  }
  else if (cfg.rollupCount != -1 || cfg.rollupTime != -1)
  {
    return new RollupFileADXLWriter();
  }
  else
  {
    return new FileADXLWriter();
  }
}

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

void FileADXLWriter::write(const AccelData& data)
{

}

void RollupFileADXLWriter::write(const AccelData& data)
{

}

