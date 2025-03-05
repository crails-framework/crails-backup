#include "tar.hpp"
#include <crails/logger.hpp>
#include <string_view>
#include <iostream>

using namespace std;
using namespace Crails;

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
    filesystem::path           backup_file = entry.path();
    filesystem::file_time_type write_time  = filesystem::last_write_time(backup_file);
    string                     filename    = backup_file.stem().string();

    filename = filename.substr(0, filename.find_first_of('.'));
#if __cplusplus >= 202002L
    list.emplace(filename, chrono::file_clock::to_sys(write_time));
#else
    // This should work, according to https://stackoverflow.com/questions/77360845/convert-stdfilesystemfile-time-type-to-epoch-nanoseconds
    // But it won't work, according to my own tests. Still, it builds with c++17.
    list.emplace(filename, chrono::system_clock::from_time_t(
      chrono::duration_cast<chrono::seconds>(write_time.time_since_epoch()).count()
    ));
#endif
  }
  return list;
}

Metadata TarBackup::read_metadata() const
{
  ostringstream command;
  filesystem::path metadata_path("crails-backup.data");

  command << "tar -zxvf " << archive_path() << ' ' << metadata_path;
  logger << "+ " << command.str() << Logger::endl;
  if (system(command.str().c_str()) == 0)
  {
    return ::read_metadata(metadata_path);
  }
  else
    logger << "failed to extract metadata from " << archive_path() << Logger::endl;
  return Metadata();
}

bool TarBackup::wipe() const
{
  return filesystem::remove(archive_path());
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
