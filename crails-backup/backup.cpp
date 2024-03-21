#include <filesystem>
#include <chrono>
#include <map>
#include <string_view>
#include <cstdlib>

using namespace std;

const char* get_backup_root()
{
  const char* backup_root = getenv("CRAILS_BACKUP_PATH");

  return backup_root ? backup_root : "/opt/crails-backup";
}

filesystem::path get_backup_folder(const string_view name)
{
  return filesystem::path(get_backup_root()) / name;
}

string archive_filename(unsigned long id)
{
  return to_string(id) + ".tar.gz";
}

map<unsigned long, filesystem::file_time_type> list_backup_archives(const string_view name)
{
  map<unsigned long, filesystem::file_time_type> list;
  filesystem::path backup_folder = get_backup_folder(name);

  filesystem::create_directories(backup_folder);
  for (const auto& entry : filesystem::directory_iterator(backup_folder))
  {
    filesystem::path backup_file = entry.path();
    auto             write_time  = filesystem::last_write_time(backup_file);
    string           filename    = backup_file.stem().string();
    unsigned long    id          = atol(filename.c_str());

    list.emplace(id, write_time);
  }
  return list;
}

void wipe_backup(const string_view name, unsigned long id)
{
  filesystem::path path = get_backup_folder(name) / archive_filename(id);

  filesystem::remove(path);
}

void wipe_expired_backups(const string_view name)
{
  using namespace chrono_literals;

  auto list = list_backup_archives(name);
  chrono::time_point now = chrono::file_clock::now();
  chrono::time_point last_time_point = now;
  chrono::time_point oldest_backup = now - (24h * 31);

  for (auto it = list.rbegin() ; it != list.rend() ; ++it)
  {
    const auto& entry = *it;

    if (entry.second < oldest_backup)
      wipe_backup(name, entry.first);
    else if ((now - entry.second) > 24h)
    {
      auto elapsed_time = (last_time_point - entry.second);

      if (elapsed_time < 24h)
        wipe_backup(name, entry.first);
      else
        last_time_point = entry.second;
    }
  }
}
