#include <crails/database_url.hpp>
#include <filesystem>
#include <sstream>
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace Crails;

static string append_connection_options(const DatabaseUrl& database)
{
  ostringstream stream;

  stream
    << " -h " << quoted(database.hostname);
  if (database.username.length() > 0)
    stream << " -U " << quoted(database.username);
  if (database.port != 0)
    stream << " -p " << database.port;
  return stream.str();
}

static void prepare_password(const DatabaseUrl& database)
{
  if (database.password.length() > 0)
    setenv("PGPASSWORD", database.password.c_str(), true);
  else
    unsetenv("PGPASSWORD");
}

bool dump_postgres(const DatabaseUrl& database, const filesystem::path& target)
{
  ostringstream stream;

  stream
    << "pg_dump --clean"
    << append_connection_options(database)
    << ' ' << quoted(database.database_name)
    << '>' << target;
  prepare_password(database);
  cout << "+ " << stream.str() << endl;
  return system(stream.str().c_str()) == 0;
}

bool restore_postgres(const DatabaseUrl& database, const filesystem::path& source)
{
  ostringstream stream;

  stream
    << "psql"
    << append_connection_options(database)
    << ' ' << quoted(database.database_name)
    << '<' << source;
  prepare_password(database);
  cout << "+ " << stream.str() << endl;
  return system(stream.str().c_str()) == 0;
}
