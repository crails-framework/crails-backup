#include <crails/logger.hpp>
#include <crails/read_file.hpp>
#include <crails/cli/command_index.hpp>
#include <crails/cli/process.hpp>
#include <filesystem>
#include <iostream>
#include "commands/add.hpp"
#include "commands/backup.hpp"
#include "commands/remove.hpp"
#include "commands/restore.hpp"
#include "commands/list.hpp"

// bzip2 might be better than gzip (slower with increased compression ?)
// -> use -j instead of -z in tar commands
// xz might be even better
// -> use -J instead of -z in tar commands

std::filesystem::path exe_path;

using namespace std;

class IndexCommand : public Crails::CommandIndex
{
public:
  IndexCommand()
  {
    add_command("backup", []() { return make_shared<BackupCommand>(); });
    add_command("restore", []() { return make_shared<RestoreCommand>(); });
    add_command("list", []() { return make_shared<ListCommand>(); });
    add_command("add", []() { return make_shared<AddCommand>(); });
    add_command("remove", []() { return make_shared<RemoveCommand>(); });
  }
};

static void find_exe_file(const char* arg0)
{
  error_code error;

  exe_path = filesystem::canonical(arg0, error);
  if (!error)
    exe_path = Crails::which(arg0);
  if (!filesystem::exists(exe_path))
    cerr << "could not find executable path, tasks related to crontab will fail" << endl;
}

int main(int argc, const char** argv)
{
  IndexCommand index;

  if (argv[0])
    find_exe_file(argv[0]);
  if (index.initialize(argc, argv))
    return index.run();
  return -1;
}
