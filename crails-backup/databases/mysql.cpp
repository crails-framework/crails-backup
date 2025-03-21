#include <crails/database_url.hpp>
#include <crails/logger.hpp>
#include <filesystem>
#include <sstream>
#include <iostream>

using namespace std;
using namespace Crails;

static string append_connection_options(const DatabaseUrl& database)
{
  ostringstream stream;

  stream << " --host=" << quoted(database.hostname);
  if (database.username.length() > 0)
    stream << " --user=" << quoted(database.username);
  if (database.password.length() > 0)
    stream << " --password=" << quoted(database.password);
  if (database.port != 0)
    stream << " --port=" << database.port;
  return stream.str();
}

bool dump_mysql(const DatabaseUrl& database, const filesystem::path& target)
{
  ostringstream stream;

  stream
    << "mysqldump"
    << append_connection_options(database)
    << ' ' << quoted(database.database_name)
    << '>' << target;
  logger << "+ " << stream.str() << Logger::endl;
  return system(stream.str().c_str()) == 0;
}

bool restore_mysql(const DatabaseUrl& database, const filesystem::path& source)
{
  ostringstream stream;

  stream
    << "mysql"
    << append_connection_options(database)
    << " -p " << quoted(database.database_name)
    << '<' << source;
  logger << "+ " << stream.str() << Logger::endl;
  return system(stream.str().c_str()) == 0;
}
