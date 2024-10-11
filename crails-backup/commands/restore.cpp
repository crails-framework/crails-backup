#include "restore.hpp"
#include "../bup.hpp"
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
    const string id = options["id"].as<string>();

    tmp_dir = filesystem::temp_directory_path() / ("restore-" + name);
    tmp_src_dir = filesystem::temp_directory_path() / ("backup-" + name);
    return restore(name, id);
  }
  return -1;
}

int RestoreCommand::restore(const string_view name, const string& id)
{
  Crails::WithPath dir_lock(tmp_dir);

  return unpack(id);
}

int RestoreCommand::unpack(const string& id)
{
  const string name = options["name"].as<string>();
  const BupBackup bup(name, id);
  const auto symbol_path_map = bup.read_metadata();

  for (const auto& entry : symbol_path_map)
  {
    const string_view symbol = entry.first;
    const string_view target = entry.second;

    if (symbol.find("database.") == 0)
      unpack_database(bup, symbol, target);
    else if (symbol.find("directory.") == 0)
      unpack_directory(bup, symbol, target);
    else if (symbol.find("file.") == 0)
      unpack_file(bup, symbol, target);
    else
      cerr << "backup metadata contains an unknown symbol: " << symbol << endl;
  }
  return 0;
}

void RestoreCommand::unpack_file(const BupBackup& bup, const string_view symbol, const filesystem::path& target)
{
  ostringstream command;

  require_parent_path(target);
  bup.restore(target, symbol);
}

void RestoreCommand::unpack_directory(const BupBackup& bup, const string_view symbol, const filesystem::path& target)
{
  Crails::WithPath change_dir("/");
  ostringstream command;

  require_parent_path(target);
  filesystem::remove_all(target);
  bup.restore(target, symbol);
}

void RestoreCommand::unpack_database(const BupBackup& bup, const string_view symbol, const string_view url)
{
  Crails::DatabaseUrl database; database.initialize(url);
  auto function = restore_functions.find(database.type);

  if (function != restore_functions.end())
  {
    stringstream tar_command;
    filesystem::path dump_path(symbol);

    if (bup.restore(dump_path, symbol))
      function->second(database, dump_path);
  }
  else
    cerr << "failed to restore database " << url << endl;
}
