#include "backup.hpp"
#include "../../backup/tar.hpp"
#include <crails/cli/with_path.hpp>
#include <sstream>
#include <iostream>

using namespace std;
using namespace Tar;

static std::filesystem::path archive_path;
static std::filesystem::path compressed_archive_path;

string filename_for_database(const Crails::DatabaseUrl& database);
filesystem::path get_backup_folder(const string_view name);
void wipe_expired_backups(BackupBase& backup);

static string tar_transformer(const string_view key, const filesystem::path& path)
{
  return "s|" + path.string().substr(1) + '|' + string(key) + '|';
}

bool BackupCommand::prepare()
{
  string name = options["name"].as<string>();

  archive_path = "/tmp/backup-" + name + ".tar";
  compressed_archive_path = archive_path.string() + ".gz";
  if (filesystem::exists(archive_path))
    filesystem::remove(archive_path);
  return BackupCommandBase::prepare();
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
  if (BackupCommandBase::pack_database(url))
  {
    Crails::WithPath path(tmp_dir);
    Crails::DatabaseUrl database(url.c_str());
    string filename = "database." + database.type + '.' + database.database_name;
    ostringstream command;

    command << add_to_archive_command_prefix()
            << ' ' << filename_for_database(database);
    cout << "+ " << command.str() << endl;
    return system(command.str().c_str()) == 0;
  }
  return false;
}

bool BackupCommand::pack_metadata()
{
  if (BackupCommandBase::pack_metadata())
  {
    ostringstream command;
    filesystem::path filename = "crails-backup.data";

    command << add_to_archive_command_prefix() << ' ' << filename;
    cout << "+ " << command.str() << endl;
    return system(command.str().c_str()) == 0;
  }
  return false;
}

bool BackupCommand::store_backup()
{
  if (gzip_pack())
  {
    stringstream     command;
    string           backup_name   = options["name"].as<string>();
    filesystem::path backup_folder = get_backup_folder(backup_name);
    filesystem::path target_path;
    TarBackup        backup(backup_name);
    auto             backup_list   = backup.list();
    auto             current_time  = chrono::file_clock::now();
    unsigned long    highest_id    = 0;

    for (const auto& backup_file : backup_list)
    {
      char* end = nullptr;
      unsigned long backup_id = strtoul(backup_file.first.c_str(), &end, 10);
      highest_id = max(backup_id, highest_id);
    }
    backup.set_backup_id(to_string(highest_id + 1));
    target_path = backup.archive_path();
    command << "mv " << compressed_archive_path << ' ' << target_path;
    cout << "+ " << command.str() << endl;
    if (system(command.str().c_str()) == 0)
    {
      wipe_expired_backups(backup);
      return true;
    }
  }
  return false;
}
