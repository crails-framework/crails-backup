#include "tar.hpp"
#include <string_view>
#include <iostream>

using namespace std;

static const map<CompressionStrategy, string> archive_extensions{
  {GzipCompression,  "gz"},
  {Bzip2Compression, "bz2"},
  {XzCompression,    "xz"}
};

TarBackup::TarBackup(const string& name, const string& id) : BackupBase(name, id)
{
}

filesystem::path TarBackup::archive_path() const
{
  const string extension = archive_extensions.at(compression_strategy());

  return path / (id + ".tar." + extension);
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

CompressionStrategy TarBackup::compression_strategy()
{
  const char* param = getenv("CRAILS_BACKUP_COMPRESSOR");

  if (param != nullptr)
  {
    string param_string(param);
    if (param_string == "bzip2")
      return Bzip2Compression;
    if (param_string == "xz")
      return XzCompression;
  }
  return GzipCompression;
}
