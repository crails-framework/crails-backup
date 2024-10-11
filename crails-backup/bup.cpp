#include "bup.hpp"
#include <crails/utils/split.hpp>
#include <crails/cli/process.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;
using namespace Crails;

filesystem::path get_backup_folder(const string_view name);

static const string_view backup_name = "crails-bup";

static string backup_date_from_id(const string_view id)
{
  ostringstream command;

  command
    << id.substr(11, 2) << ':'
    << id.substr(13, 2) << ' '
    << id.substr(0, 4) << '-'
    << id.substr(5, 2) << '-'
    << id.substr(8, 2);
  return command.str();
}

BupBackup::BupBackup(const string& name, const string& id) : name(name), path(get_backup_folder(name)), id(id)
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

bool BupBackup::init() const
{
  return std::system("bup init") == 0;
}

bool BupBackup::index(const filesystem::path& target) const
{
  ostringstream command;

  command << "bup index " << target;
  cout << "+ " << command.str() << endl;
  return std::system(command.str().c_str()) == 0;
}

bool BupBackup::save(const filesystem::path& target) const
{
  ostringstream command;

  command << "bup save -n " << backup_name << ' ' << target;
  cout << "+ " << command.str() << endl;
  return std::system(command.str().c_str()) == 0;
}

bool BupBackup::restore(const filesystem::path& target, const string_view key) const
{
  ostringstream command, mv_command;
  filesystem::path bup_path(key);

  command << "bup restore "
    << "-C . "
    << path_prefix() << '/' << key;
  cout << "+ " << command.str() << endl;
  if (std::system(command.str().c_str()) == 0)
  {
    filesystem::rename(bup_path.filename(), target);
    return true;
  }
  return false;
}

bool BupBackup::list(ListMode mode) const
{
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
      switch (mode)
      {
      case ShortMode:
        cout << backup_id << endl;
        break ;
      case LongMode:
        cout
          << left << setw(20) << backup_id << ' '
          << backup_date_from_id(
               backup_id == "latest" ? latest_backup : backup_id)
          << endl;
        break ;
      }
    }
    return true;
  }
  return false;
}

Metadata BupBackup::read_metadata() const
{
  Metadata results;
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
  {
    ifstream stream(metadata_path);

    if (stream.is_open())
    {
      string line_symbol;
      string line_path;

      while (getline(stream, line_symbol) && getline(stream, line_path))
        results.emplace(line_symbol, line_path);
    }
  }
  filesystem::remove(metadata_path);
  return results;
}
