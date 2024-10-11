#include "restore.hpp"
#include "../../backup/bup.hpp"
#include <crails/cli/with_path.hpp>
#include <cstdlib>
#include <iostream>
#include <fstream>

using namespace std;
using namespace Bup;

static bool bup_restore(const BupBackup& backup, const filesystem::path& target, const string_view key)
{
  ostringstream command, mv_command;
  filesystem::path bup_path(key);

  command << "bup restore "
    << "-C . "
    << backup.path_prefix() << '/' << key;
  cout << "+ " << command.str() << endl;
  if (std::system(command.str().c_str()) == 0)
  {
    filesystem::rename(bup_path.filename(), target);
    return true;
  }
  return false;
}

int RestoreCommand::restore(const string_view name, const string& id)
{
  Crails::WithPath dir_lock(tmp_dir);

  return unpack(id);
}

int RestoreCommand::unpack(const string& id)
{
  const string name = options["name"].as<string>();
  const BupBackup bup(name, id);
  const auto symbol_path_map = bup.read_metadata();

  for (const auto& entry : symbol_path_map)
  {
    const string_view symbol = entry.first;
    const string_view target = entry.second;

    if (symbol.find("database.") == 0)
      unpack_database(bup, symbol, target);
    else if (symbol.find("directory.") == 0)
      unpack_directory(bup, symbol, target);
    else if (symbol.find("file.") == 0)
      unpack_file(bup, symbol, target);
    else
      cerr << "backup metadata contains an unknown symbol: " << symbol << endl;
  }
  return 0;
}

void RestoreCommand::unpack_file(const BupBackup& bup, const string_view symbol, const filesystem::path& target)
{
  ostringstream command;

  require_parent_path(target);
  bup_restore(bup, target, symbol);
}

void RestoreCommand::unpack_directory(const BupBackup& bup, const string_view symbol, const filesystem::path& target)
{
  Crails::WithPath change_dir("/");
  ostringstream command;

  require_parent_path(target);
  filesystem::remove_all(target);
  bup_restore(bup, target, symbol);
}

void RestoreCommand::unpack_database(const BupBackup& bup, const string_view symbol, const string_view url)
{
  Crails::DatabaseUrl database; database.initialize(url);
  auto function = restore_functions.find(database.type);

  if (function != restore_functions.end())
  {
    stringstream tar_command;
    filesystem::path dump_path(symbol);

    if (bup_restore(bup, dump_path, symbol))
      function->second(database, dump_path);
  }
  else
    cerr << "failed to restore database " << url << endl;
}
