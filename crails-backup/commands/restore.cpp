#include "restore.hpp"
#include <crails/cli/with_path.hpp>
#include <crails/database_url.hpp>
#include <filesystem>
#include <cstdlib>
#include <iostream>
#include <fstream>

using namespace std;

filesystem::path get_backup_folder(const string_view name);

typedef bool (*DatabaseRestoreFunction)(const Crails::DatabaseUrl&, const filesystem::path&);
typedef map<string_view, DatabaseRestoreFunction> RestoreFunctionMap;

bool restore_mysql(const Crails::DatabaseUrl&, const filesystem::path&);
bool restore_postgres(const Crails::DatabaseUrl&, const filesystem::path&);
bool restore_mongodb(const Crails::DatabaseUrl&, const filesystem::path&);

RestoreFunctionMap restore_functions = {
  {"mysql",    &restore_mysql},
  {"postgres", &restore_postgres},
  {"mongodb",  &restore_mongodb}
};

static void require_parent_path(const filesystem::path& path)
{
  filesystem::path parent_path = path.parent_path();

  if (!filesystem::exists(parent_path))
  {
    filesystem::create_directories(parent_path);
  }
  else if (!filesystem::is_directory(parent_path))
  {
    throw runtime_error("path " + path.string() + " was expected to be a directory, but it isn't.");
  }
}

static string tar_transformer(const string_view key, const filesystem::path& path)
{
  return "s|" + string(key) + '|' + path.string().substr(1) + '|';
}

RestoreCommand::~RestoreCommand()
{
  if (filesystem::is_directory(tmp_dir))
    filesystem::remove_all(tmp_dir);
}

void RestoreCommand::options_description(boost::program_options::options_description& desc) const
{
  desc.add_options()
    ("name,n", boost::program_options::value<std::string>(), "backup name")
    ("id",     boost::program_options::value<std::string>(), "backup id");
}

int RestoreCommand::run()
{
  if (options.count("name") && options.count("id"))
  {
    const string name = options["name"].as<string>();
    unsigned long id = std::atol(options["id"].as<string>().c_str());

    tmp_dir = filesystem::temp_directory_path() / ("restore-" + name);
    return restore(name, id);
  }
  return -1;
}

int RestoreCommand::restore(const string_view name, unsigned long id)
{
  filesystem::path backup_folder = get_backup_folder(name);
  filesystem::path archive_path = backup_folder / (to_string(id) + ".tar.gz");

  if (filesystem::is_regular_file(archive_path))
  {
    Crails::WithPath dir_lock(tmp_dir);

    return unpack(archive_path);
  }
  else
    cerr << "backup archive " << archive_path << " not found." << endl;
  return -1;
}

int RestoreCommand::unpack(const filesystem::path& archive_path)
{
  auto symbol_path_map = read_metadata(archive_path);

  for (const auto& entry : symbol_path_map)
  {
    const string_view symbol = entry.first;
    const string_view target = entry.second;

    if (symbol.find("database.") == 0)
      unpack_database(archive_path, symbol, target);
    else if (symbol.find("directory.") == 0)
      unpack_directory(archive_path, symbol, target);
    else if (symbol.find("file.") == 0)
      unpack_file(archive_path, symbol, target);
    else
      cerr << "backup metadata contains an unknown symbol: " << symbol << endl;
  }
  return 0;
}

void RestoreCommand::unpack_file(const filesystem::path& archive_path, const string_view symbol, const filesystem::path& target)
{
  ostringstream command;

  require_parent_path(target);
  command << "tar -zxvf " << archive_path
          << ' ' << symbol
          << " -O>" << target;
  cout << "+ " << command.str() << endl;
  system(command.str().c_str());
}

void RestoreCommand::unpack_directory(const filesystem::path& archive_path, const string_view symbol, const filesystem::path& target)
{
  Crails::WithPath change_dir("/");
  ostringstream command;

  require_parent_path(target);
  command << "tar -zxvf " << archive_path
          << " --transform " << quoted(tar_transformer(symbol, target))
          << ' ' << symbol;
  cout << "+ " << command.str() << endl;
  system(command.str().c_str());
}

void RestoreCommand::unpack_database(const filesystem::path& archive_path, const string_view symbol, const string_view url)
{
  Crails::DatabaseUrl database; database.initialize(url);
  auto function = restore_functions.find(database.type);

  if (function != restore_functions.end())
  {
    stringstream tar_command;
    filesystem::path dump_path(symbol);

    tar_command << "tar -zxvf " << archive_path << ' ' << symbol;
    cout << "+ " << tar_command.str() << endl;
    if (system(tar_command.str().c_str()) == 0)
      function->second(database, dump_path);
  }
  else
    cerr << "failed to restore database " << url << endl;
}

std::map<std::string,std::string> RestoreCommand::read_metadata(const std::filesystem::path& archive_path)
{
  stringstream command;
  filesystem::path metadata_path("crails-backup.data");
  map<string,string> results;

  command << "tar -zxvf " << archive_path << ' ' << metadata_path;
  cout << "+ " << command.str() << endl;
  if (system(command.str().c_str()) == 0)
  {
    ifstream stream(metadata_path);

    if (stream.is_open())
    {
      string line_symbol;
      string line_path;

      while (getline(stream, line_symbol) && getline(stream, line_path))
        results.emplace(line_symbol, line_path);
    }
    else
      cerr << "failed to open " << metadata_path << endl;
  }
  else
    cerr << "failed to extract metadata from " << archive_path << endl;
  return results;
}
