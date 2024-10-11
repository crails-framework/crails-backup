#include "tar.hpp"
#include <string_view>
#include <iostream>

using namespace std;

TarBackup::TarBackup(const string& name, const string& id) : BackupBase(name, id)
{
}

filesystem::path TarBackup::archive_path() const
{
  return path / (id + ".tar.gz");
}

BackupList TarBackup::list() const
{
  BackupList list;

  filesystem::create_directories(path);
  for (const auto& entry : filesystem::directory_iterator(path))
  {
    filesystem::path backup_file = entry.path();
    auto             write_time  = filesystem::last_write_time(backup_file);
    string           filename    = backup_file.stem().string();

    list.emplace(filename, chrono::file_clock::to_sys(write_time));
  }
  return list;
}

Metadata TarBackup::read_metadata() const
{
  ostringstream command;
  filesystem::path metadata_path("crails-backup.data");

  command << "tar -zxvf " << archive_path() << ' ' << metadata_path;
  cout << "+ " << command.str() << endl;
  if (system(command.str().c_str()) == 0)
  {
    return ::read_metadata(metadata_path);
  }
  else
    cerr << "failed to extract metadata from " << archive_path() << endl;
  return Metadata();
}

bool TarBackup::wipe() const
{
  filesystem::remove(archive_path());
}
