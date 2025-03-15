#include <crails/database_url.hpp>
#include <crails/logger.hpp>
#include <filesystem>
#include <sstream>
#include <iostream>

using namespace std;
using namespace Crails;

bool dump_mongodb(const DatabaseUrl& database, const filesystem::path& target)
{
  ostringstream stream;

  stream
    << "mongodump"
    << " --archive=" << target
    << " --uri=" << database.to_string();
  logger << "+ " << stream.str() << Logger::endl;
  return system(stream.str().c_str()) == 0;
}

bool restore_mongodb(const DatabaseUrl& database, const filesystem::path& target)
{
  ostringstream stream;

  stream
    << "mongorestore"
    << " --archive=" << target
    << " --uri=" << database.to_string()
    << " --drop";
  logger << "+ " << stream.str() << Logger::endl;
  return system(stream.str().c_str()) == 0;
}
