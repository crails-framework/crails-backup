#include "bup.hpp"
#include <crails/logger.hpp>
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

const char* get_backup_root();

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
    "bup", vector<string>{"ls", "-n", id}
  };
  string output;

  append_server_options(command);
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
    "bup", {"restore"}
  };
  filesystem::path metadata_path;

  append_server_options(command);
  metadata_path = filesystem::canonical(".") / "crails-backup.data";
  command
    << "-C" << metadata_path.parent_path().string()
    << (path_prefix() + "/crails-backup.data");
  logger << "BupBackup::read_metadata: " << command << Logger::endl;
  run_command(command);
  return ::read_metadata(metadata_path);
}

bool BupBackup::wipe() const
{
  ExecutableCommand command{
    "bup", {"rm", "--unsafe"}
  };

  append_server_options(command);
  command << path_prefix();
  return run_command(command);
}

static string server_option(const string_view hostname)
{
  ostringstream stream;

  stream << hostname << ':' << string_view(get_backup_root()).substr(1);
  return stream.str();
}

void BupBackup::append_server_options(Crails::ExecutableCommand& command)
{
  const char* hostname = getenv("CRAILS_BACKUP_SERVERNAME");

  if (hostname)
    command << "-r" << server_option(hostname);
}
