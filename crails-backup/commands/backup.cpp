#include "backup.hpp"
#include <crails/cli/with_path.hpp>
#include <fstream>
#include <iostream>

using namespace std;

bool dump_mysql(const Crails::DatabaseUrl&, const filesystem::path&);
bool dump_postgres(const Crails::DatabaseUrl&, const filesystem::path&);
bool dump_mongodb(const Crails::DatabaseUrl&, const filesystem::path&);

const DatabaseBackupFunctionMap BackupCommandBase::dump_functions = {
  {"mysql",    &dump_mysql},
  {"postgres", &dump_postgres},
  {"mongodb",  &dump_mongodb}
};

string filename_for_database(const Crails::DatabaseUrl& database)
{
  return "database." + database.type + '.' + database.database_name;
}

BackupCommandBase::~BackupCommandBase()
{
  if (filesystem::exists(tmp_dir))
    filesystem::remove_all(tmp_dir);
}

static pair<string_view,string_view> make_storage_id(const string_view text)
{
  int separator = text.find(':');

  if (separator == string::npos)
    return {"", text};
  return {text.substr(0, separator), text.substr(separator + 1)};
}

void BackupCommandBase::options_description(boost::program_options::options_description& desc) const
{
  using namespace boost;
  desc.add_options()
    ("name,n",     program_options::value<string>(),         "backup name")
    ("database,d", program_options::value<vector<string>>(), "list of database urls")
    ("file,f",     program_options::value<vector<string>>(), "list of file and directories");
}

bool BackupCommandBase::prepare()
{
  string name = options["name"].as<string>();

  tmp_dir = filesystem::temp_directory_path() / ("backup-" + name);
  return true;
}

int BackupCommandBase::run()
{
  if (options.count("name") && prepare())
  {
    if (options.count("database"))
    {
      for (const auto& url : options["database"].as<vector<string>>())
      {
        if (!pack_database(url))
          return -2;
      }
    }
    if (options.count("file"))
    {
      for (const auto& path : options["file"].as<vector<string>>())
      {
        auto storage_id = make_storage_id(path);

        if (!storage_id.first.length())
          storage_id.first = storage_id.second;
        if (!(pack_path(storage_id.first, filesystem::path(storage_id.second))))
          return -2;
      }
    }
    if (pack_metadata() && store_backup())
      return 0;
  }
  return -1;
}

bool BackupCommandBase::pack_metadata()
{
  Crails::WithPath path(tmp_dir);
  filesystem::path filename = "crails-backup.data";
  ofstream stream(filename);
  stringstream command;

  if (stream.is_open())
  {
    for (auto it = name_to_path_map.begin() ; it != name_to_path_map.end() ; ++it)
      stream << it->first << '\n' << it->second << '\n';
    stream.close();
    return true;
  }
  return false;
}

bool BackupCommandBase::pack_path(const string_view key, const filesystem::path& path)
{
  if (filesystem::exists(path))
  {
    filesystem::path absolute_path = filesystem::canonical(path);
    Crails::WithPath current_path("/");
    stringstream command;
    string full_key;

    if (filesystem::is_directory(absolute_path))
      full_key = "directory." + string(key);
    else
      full_key = "file." + string(key);
    name_to_path_map.emplace(full_key, absolute_path.string());
    filesystem::create_directories((tmp_dir / full_key).parent_path());
    filesystem::copy(
      absolute_path,
      tmp_dir / full_key,
      filesystem::copy_options::recursive
    );
    return true;
  }
  else
    cerr << "crails-backup could not solve " << path << endl;
  return true;
}

bool BackupCommandBase::pack_database(const string& url)
{
  Crails::DatabaseUrl database(url.c_str());
  auto it = dump_functions.find(database.type);

  if (it != dump_functions.end())
  {
    Crails::WithPath path(tmp_dir);
    string filename = filename_for_database(database);
    filesystem::path dump_path = tmp_dir / filename;

    it->second(database, dump_path);
    if (filesystem::is_regular_file(dump_path))
    {
      name_to_path_map.emplace(filename, url);
      return true;
    }
    else
      cerr << "dump failed for database " << url << endl;
  }
  else
    cerr << "usupported database type " << database.type << endl;
  return false;
}
