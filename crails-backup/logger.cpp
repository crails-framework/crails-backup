#include "logger.hpp"
#include <crails/logger.hpp>
#include <iostream>

using namespace std;
using namespace Crails;

static void display_command(int argc, const char** argv)
{
  logger << ":: Running: ";
  for (int i = 0 ; i < argc ; ++i)
    logger << argv[i] << ' ';
  logger << Logger::endl;
}

LoggingFile::LoggingFile(int argc, const char** argv)
{
  const char* output = getenv("CRAILS_BACKUP_LOG");

  if (output != nullptr)
  {
    stream = new ofstream(output, ios_base::app);
    if (stream->is_open())
    {
      logger.set_stdout(*stream);
      logger.set_stderr(*stream);
      display_command(argc, argv);
      return ;
    }
    else
      cerr << "Failed to open logfile " << output << endl;
    delete stream;
  }
  stream = nullptr;
}

LoggingFile::~LoggingFile()
{
  if (stream)
  {
    logger.set_stdout(cout);
    logger.set_stderr(cerr);
    stream->close();
    delete stream;
  }
}
