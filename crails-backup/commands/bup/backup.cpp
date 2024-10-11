#include <crails/cli/with_path.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include "backup.hpp"
#include "../../backup/bup.hpp"

using namespace std;
using namespace Bup;

filesystem::path get_backup_folder(const string_view name);
void wipe_expired_backups(BackupBase& backup);

static bool bup_init()
{
  return std::system("bup init") == 0;
}

static bool bup_index(const filesystem::path& target)
{
  ostringstream command;

  command << "bup index " << target;
  cout << "+ " << command.str() << endl;
  return std::system(command.str().c_str()) == 0;
}

static bool bup_save(const string_view backup_name, const filesystem::path& target)
{
  ostringstream command;

  command << "bup save -n " << backup_name << ' ' << target;
  cout << "+ " << command.str() << endl;
  return std::system(command.str().c_str()) == 0;
}

bool BackupCommand::store_backup()
{
  string           backup_name   = options["name"].as<string>();
  filesystem::path backup_folder = get_backup_folder(backup_name);
  BupBackup        bup(backup_name);

  if (bup_init() && bup_index(tmp_dir) && bup_save(backup_name, tmp_dir))
  {
    wipe_expired_backups(bup);
    return true;
  }
  return false;
}
