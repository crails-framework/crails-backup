#include "restore.hpp"
#include "../../backup/tar.hpp"
#include <iostream>
#include <crails/cli/with_path.hpp>

using namespace std;
using namespace Tar;

static string tar_transformer(const string_view key, const filesystem::path& path)
{
  return "s|" + string(key) + '|' + path.string().substr(1) + '|';
}

int RestoreCommand::restore(const string_view name, const string& id)
{
  const TarBackup backup(string(name), id);

  if (filesystem::is_regular_file(backup.archive_path()))
  {
    Crails::WithPath dir_lock(tmp_dir);

    return unpack(backup);
  }
  else
    cerr << "backup archive " << backup.archive_path() << " not found." << endl;
  return -1;
}

int RestoreCommand::unpack(const TarBackup& backup)
{
  const auto symbol_path_map = backup.read_metadata();

  for (const auto& entry : symbol_path_map)
  {
    const string_view symbol = entry.first;
    const string_view target = entry.second;

    if (symbol.find("database.") == 0)
      unpack_database(backup, symbol, target);
    else if (symbol.find("directory.") == 0)
      unpack_directory(backup, symbol, target);
    else if (symbol.find("file.") == 0)
      unpack_file(backup, symbol, target);
    else
      cerr << "backup metadata contains an unknown symbol: " << symbol << endl;
  }
  return 0;
}

void RestoreCommand::unpack_file(const TarBackup& backup, const string_view symbol, const filesystem::path& target)
{
  ostringstream command;

  require_parent_path(target);
  command << "tar -zxvf " << backup.archive_path()
          << ' ' << symbol
          << " -O>" << target;
  cout << "+ " << command.str() << endl;
  system(command.str().c_str());
}

void RestoreCommand::unpack_directory(const TarBackup& backup, const string_view symbol, const filesystem::path& target)
{
  Crails::WithPath change_dir("/");
  ostringstream command;

  require_parent_path(target);
  command << "tar -zxvf " << backup.archive_path()
          << " --transform " << quoted(tar_transformer(symbol, target))
          << ' ' << symbol;
  cout << "+ " << command.str() << endl;
  system(command.str().c_str());
}

void RestoreCommand::unpack_database(const TarBackup& backup, const string_view symbol, const string_view url)
{
  Crails::DatabaseUrl database; database.initialize(url);
  auto function = restore_functions.find(database.type);

  if (function != restore_functions.end())
  {
    stringstream tar_command;
    filesystem::path dump_path(symbol);

    tar_command << "tar -zxvf " << backup.archive_path() << ' ' << symbol;
    cout << "+ " << tar_command.str() << endl;
    if (system(tar_command.str().c_str()) == 0)
      function->second(database, dump_path);
  }
  else
    cerr << "failed to restore database " << url << endl;
}
