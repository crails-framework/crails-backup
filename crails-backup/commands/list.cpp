#include "list.hpp"
#include "../backup/base.hpp"
#include <filesystem>
#include <iostream>

using namespace std;

const char* get_backup_root();
  
void ListCommandBase::options_description(boost::program_options::options_description& desc) const
{
  desc.add_options()
    ("name,n", boost::program_options::value<std::string>(), "backup name")
    ("short,s", "displays backup ids only");
}

int ListCommandBase::run()
{
  return options.count("name")
       ? list_files()
       : list_backups();
}

int ListCommandBase::list_backups()
{
  filesystem::path backup_root = get_backup_root();

  if (filesystem::exists(backup_root))
  {
    for (const auto& entry : filesystem::directory_iterator(backup_root))
      cout << entry.path().filename().string() << endl;
  }
  return 0;
}

int ListCommandBase::list_files(const BackupBase& backup)
{
  const auto archives = backup.list();
  bool short_mode = options.count("short");

  for (const auto& archive : archives)
  {
    if (short_mode)
      cout << archive.first << endl;
    else
      cout << left << setw(20) << archive.first << ' ' << format("{:%H:%M %Y-%m-%d}", archive.second) << endl;
  }
  return 0;
}
