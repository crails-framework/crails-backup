#include <crails/database_url.hpp>
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

void dump_mysql(const DatabaseUrl& database, const filesystem::path& target)
{
  ostringstream stream;

  stream
    << "mysqldump"
    << append_connection_options(database)
    << ' ' << quoted(database.database_name)
    << '>' << target;
  cout << "+ " << stream.str() << endl;
  system(stream.str().c_str());
}

void restore_mysql(const DatabaseUrl& database, const filesystem::path& source)
{
  ostringstream stream;

  stream
    << "mysql"
    << append_connection_options(database)
    << " -p " << quoted(database.database_name)
    << '<' << source;
  cout << "+ " << stream.str() << endl;
  system(stream.str().c_str());
}
