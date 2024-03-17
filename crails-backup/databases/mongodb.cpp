#include <crails/database_url.hpp>
#include <filesystem>
#include <sstream>
#include <iostream>

using namespace std;
using namespace Crails;

void dump_mongodb(const DatabaseUrl& database, const filesystem::path& target)
{
  ostringstream stream;

  stream
    << "mongodump"
    << " --archive " << target
    << " --uri " << database.to_string();
  cout << "+ " << stream.str() << endl;
  system(stream.str().c_str());
}

void restore_mongodb(const DatabaseUrl& database, const filesystem::path& target)
{
  ostringstream stream;

  stream
    << "mongorestore"
    << " --uri " << database.to_string()
    << ' ' << target;
  cout << "+ " << stream.str() << endl;
  system(stream.str().c_str());
}
