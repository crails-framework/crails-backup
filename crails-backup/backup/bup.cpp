#include "bup.hpp"
#include <crails/utils/split.hpp>
#include <crails/cli/process.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>

using namespace std;
using namespace Crails;

static const string_view backup_name = "crails-bup";

static chrono::system_clock::time_point backup_date_from_id(const string id)
{
  tm t = {};
  istringstream ss(id.c_str());
  ss >> get_time(&t, "%Y-%m-%d-%H%M%S");
  return chrono::system_clock::from_time_t(mktime(&t));
}

BupBackup::BupBackup(const string& name, const string& id) : BackupBase(name, id)
{
  setenv("BUP_DIR", path.string().c_str(), 1);
}

string BupBackup::path_prefix() const
{
  ostringstream stream;
  filesystem::path tmp_dir = filesystem::temp_directory_path() / ("backup-" + name);

  stream << backup_name << '/' << id << tmp_dir.string();
  return stream.str();
}

BackupList BupBackup::list() const
{
  BackupList results;
  ExecutableCommand command{
    "bup", vector<string>{"ls", "-n", string(backup_name)}
  };
  string output;

  if (run_command(command, output))
  {
    auto backups = split(output, '\n');
    string latest_backup;

    if (backups.size() > 1)
      latest_backup = *(++backups.rbegin());
    for (const string& backup_id : backups)
    {
      auto date = backup_date_from_id(
        backup_id == "latest" ? latest_backup : backup_id
      );

      results.emplace(backup_id, date);
    }
  }
  return results;
}

Metadata BupBackup::read_metadata() const
{
  ExecutableCommand command{
    "bup", {"restore", "-C"}
  };
  filesystem::path metadata_path;

  metadata_path = filesystem::canonical(".") / "crails-backup.data";
  command
    << metadata_path.parent_path().string()
    << (path_prefix() + "/crails-backup.data");
  cout << "BupBackup::read_metadata: bup restore -C " << metadata_path.string() << ' ' << path_prefix() << "/crails-backup.data" << endl;
  run_command(command);
  return ::read_metadata(metadata_path);
}

bool BupBackup::wipe() const
{
  ExecutableCommand command{
    "bup", {"rm", "--unsafe"}
  };

  command << path_prefix();
  return run_command(command);
}
