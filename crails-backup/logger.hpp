#pragma once
#include <fstream>

class LoggingFile
{
public:
  LoggingFile(int argc, const char** argv);
  ~LoggingFile();

private:
  std::ofstream* stream;
};
