#include <filesystem>
#include <chrono>
#include <map>
#include <string_view>
#include <cstdlib>
#include "backup/base.hpp"

using namespace std;

chrono::duration<int> duration_from_env(const char* varname, chrono::duration<int> default_value);

const char* get_backup_root()
{
  const char* backup_root = getenv("CRAILS_BACKUP_PATH");

  return backup_root ? backup_root : "/opt/crails-backup";
}

filesystem::path get_backup_folder(const string_view name)
{
  return filesystem::path(get_backup_root()) / name;
}

void wipe_expired_backups(BackupBase& backup)
{
  using namespace chrono_literals;

  const chrono::duration<int> maximum_backup_retention   = duration_from_env("BACKUP_MAX_RETENTION", 24h * 31);
  const chrono::duration<int> long_retention_start       = duration_from_env("BACKUP_LONGTERM_STARTS_AFTER", 24h);
  const chrono::duration<int> long_retention_periodicity = duration_from_env("BACKUP_LONGTERM_PERIODICITY", 24h);

  auto list = backup.list();
  chrono::time_point now = chrono::system_clock::now();
  chrono::time_point last_time_point = now;
  chrono::time_point oldest_backup = now - maximum_backup_retention;

  for (auto it = list.rbegin() ; it != list.rend() ; ++it)
  {
    const auto& entry = *it;

    backup.set_backup_id(entry.first);
    if (entry.second < oldest_backup)
      backup.wipe();
    else if ((now - entry.second) > long_retention_start)
    {
      auto elapsed_time = (last_time_point - entry.second);

      if (elapsed_time < long_retention_periodicity)
        backup.wipe();
      else
        last_time_point = entry.second;
    }
  }
}
