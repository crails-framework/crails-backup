#include <crails/cli/with_path.hpp>
#include <crails/cli/process.hpp>
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
  Crails::ExecutableCommand command{
    "bup", {"init"}
  };
  
  BupBackup::append_server_options(command);
  return Crails::run_command(command);
}

static bool bup_index(const filesystem::path& target)
{
  Crails::ExecutableCommand command{
    "bup", {"index"}
  };

  BupBackup::append_server_options(command);
  command << target.string();
  return Crails::run_command(command);
}

static bool bup_save(const string_view backup_name, const filesystem::path& target)
{
  Crails::ExecutableCommand command{
    "bup", {"save"}
  };

  BupBackup::append_server_options(command);
  command << "-n" << backup_name << target.string();
  return Crails::run_command(command);
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
