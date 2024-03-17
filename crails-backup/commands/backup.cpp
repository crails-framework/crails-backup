#include <crails/database_url.hpp>
#include <crails/cli/with_path.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include "backup.hpp"

using namespace std;

static std::filesystem::path archive_path;
static std::filesystem::path compressed_archive_path;
static std::filesystem::path tmp_dir;
static std::map<std::string, std::string> name_to_path_map;

filesystem::path get_backup_folder(const string_view name);
map<unsigned long, filesystem::file_time_type> list_backup_archives(const string_view name);
string archive_filename(unsigned long id);
void wipe_expired_backups(const string_view name);

typedef void (*DatabaseDumpFunction)(const Crails::DatabaseUrl&, const filesystem::path&);

void dump_mysql(const Crails::DatabaseUrl&, const filesystem::path&);
void dump_postgres(const Crails::DatabaseUrl&, const filesystem::path&);
void dump_mongodb(const Crails::DatabaseUrl&, const filesystem::path&);

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

static string tar_transformer(const string_view key, const filesystem::path& path)
{
  return "s|" + path.string().substr(1) + '|' + string(key) + '|';
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
    if (pack_metadata() && gzip_pack() && store_backup())
      return 0;
  }
  return -1;
}

bool BackupCommand::store_backup()
{
  stringstream     command;
  string           backup_name   = options["name"].as<string>();
  filesystem::path backup_folder = get_backup_folder(backup_name);
  filesystem::path target_path;
  auto             backup_list   = list_backup_archives(backup_name);
  auto             current_time  = chrono::file_clock::now();
  unsigned long    highest_id    = 0;

  for (const pair<unsigned long, filesystem::file_time_type>& backup_file : backup_list)
    highest_id = max(backup_file.first, highest_id);
  target_path = backup_folder / archive_filename(highest_id + 1);
  command << "mv " << compressed_archive_path << ' ' << target_path;
  cout << "+ " << command.str() << endl;
  if (system(command.str().c_str()) == 0)
  {
    wipe_expired_backups(backup_name);
    return true;
  }
  return false;
}

string BackupCommand::add_to_archive_command_prefix() const
{
  ostringstream stream;

  stream << "tar -rf " << archive_path;
  return stream.str();
}

bool BackupCommand::gzip_pack()
{
  ostringstream stream;

  stream << "gzip " << archive_path;
  return system(stream.str().c_str()) == 0;
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
    command << add_to_archive_command_prefix() << ' ' << filename;
    cout << "+ " << command.str() << endl;
    return system(command.str().c_str()) == 0;
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
    command << add_to_archive_command_prefix()
            << " --transform "
            << quoted(tar_transformer(full_key, absolute_path))
            << ' ' << absolute_path;
    cout << "+ " << command.str() << endl;
    return system(command.str().c_str()) == 0;
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
      stringstream command;

      name_to_path_map.emplace(filename, url);
      command << add_to_archive_command_prefix()
              << ' ' << filename;
      cout << "+ " << command.str() << endl;
      return system(command.str().c_str()) == 0;
    }
    else
      cerr << "dump failed for database " << url << endl;
  }
  else
    cerr << "usupported database type " << database.type << endl;
  return false;
}
