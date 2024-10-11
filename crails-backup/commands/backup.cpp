#include <crails/database_url.hpp>
#include <crails/cli/with_path.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include "backup.hpp"
#include "../bup.hpp"

using namespace std;

static std::filesystem::path archive_path;
static std::filesystem::path compressed_archive_path;
static std::filesystem::path tmp_dir;
static std::map<std::string, std::string> name_to_path_map;

filesystem::path get_backup_folder(const string_view name);
void wipe_expired_backups(const string_view name);

typedef bool (*DatabaseDumpFunction)(const Crails::DatabaseUrl&, const filesystem::path&);

bool dump_mysql(const Crails::DatabaseUrl&, const filesystem::path&);
bool dump_postgres(const Crails::DatabaseUrl&, const filesystem::path&);
bool dump_mongodb(const Crails::DatabaseUrl&, const filesystem::path&);

map<string_view, DatabaseDumpFunction> dump_functions = {
  {"mysql",    &dump_mysql},
  {"postgres", &dump_postgres},
  {"mongodb",  &dump_mongodb}
};

static pair<string_view,string_view> make_storage_id(const string_view text)
{
  int separator = text.find(':');

  if (separator == string::npos)
    return {"", text};
  return {text.substr(0, separator), text.substr(separator + 1)};
}

BackupCommand::~BackupCommand()
{
  if (filesystem::exists(tmp_dir))
    filesystem::remove_all(tmp_dir);
}

void BackupCommand::options_description(boost::program_options::options_description& desc) const
{
  using namespace boost;
  desc.add_options()
    ("name,n",     program_options::value<string>(),         "backup name")
    ("database,d", program_options::value<vector<string>>(), "list of database urls")
    ("file,f",     program_options::value<vector<string>>(), "list of file and directories");
}

int BackupCommand::run()
{
  if (options.count("name"))
  {
    string name = options["name"].as<string>();

    archive_path = "/tmp/backup-" + name + ".tar";
    compressed_archive_path = archive_path.string() + ".gz";
    tmp_dir = filesystem::temp_directory_path() / ("backup-" + name);
    if (filesystem::exists(archive_path))
      filesystem::remove(archive_path);
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

bool BackupCommand::store_backup()
{
  string           backup_name   = options["name"].as<string>();
  filesystem::path backup_folder = get_backup_folder(backup_name);
  const BupBackup  bup(backup_name);

  if (bup.init() && bup.index(tmp_dir) && bup.save(tmp_dir))
  {
    wipe_expired_backups(backup_name);
    return true;
  }
  return false;
}

bool BackupCommand::pack_metadata()
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

bool BackupCommand::pack_path(const string_view key, const filesystem::path& path)
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

bool BackupCommand::pack_database(const string& url)
{
  Crails::DatabaseUrl database(url.c_str());
  auto it = dump_functions.find(database.type);

  if (it != dump_functions.end())
  {
    Crails::WithPath path(tmp_dir);
    string filename = "database." + database.type + '.' + database.database_name;
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
