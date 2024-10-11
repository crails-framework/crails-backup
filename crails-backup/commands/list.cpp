#include "list.hpp"
#include "../bup.hpp"
#include <filesystem>
#include <string_view>
#include <sstream>
#include <iomanip>
#include <format>
#include <chrono>
#include <iostream>

using namespace std;

const char* get_backup_root();
filesystem::path get_backup_folder(const string_view name);
  
void ListCommand::options_description(boost::program_options::options_description& desc) const
{
  desc.add_options()
    ("name,n", boost::program_options::value<std::string>(), "backup name")
    ("short,s", "displays backup ids only");
}

int ListCommand::run()
{
  return options.count("name")
       ? list_files()
       : list_backups();
}

int ListCommand::list_backups()
{
  filesystem::path backup_root = get_backup_root();

  if (filesystem::exists(backup_root))
  {
    for (const auto& entry : filesystem::directory_iterator(backup_root))
      cout << entry.path().filename().string() << endl;
  }
  return 0;
}

int ListCommand::list_files()
{
  const string backup_name = options["name"].as<string>();
  filesystem::path backup_folder = get_backup_folder(backup_name);
  const BupBackup backup(backup_folder);
  ListMode mode = options.count("short") ? ShortMode : LongMode;

  return backup.list(mode) ? 0 : -1;
}
