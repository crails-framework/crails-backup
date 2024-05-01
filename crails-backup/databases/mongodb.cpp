#include <crails/database_url.hpp>
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
    << " --archive " << target
    << " --uri " << database.to_string();
  cout << "+ " << stream.str() << endl;
  return system(stream.str().c_str()) == 0;
}

bool restore_mongodb(const DatabaseUrl& database, const filesystem::path& target)
{
  ostringstream stream;

  stream
    << "mongorestore"
    << " --uri " << database.to_string()
    << ' ' << target;
  cout << "+ " << stream.str() << endl;
  return system(stream.str().c_str()) == 0;
}
