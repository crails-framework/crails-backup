#include <crails/logger.hpp>
#include <crails/read_file.hpp>
#include <crails/cli/command_index.hpp>
#include <crails/cli/process.hpp>
#include <filesystem>
#include <iostream>
#include "commands/add.hpp"
#include "commands/tar/backup.hpp"
#include "commands/tar/restore.hpp"
#include "commands/tar/list.hpp"
#include "commands/bup/backup.hpp"
#include "commands/bup/restore.hpp"
#include "commands/bup/list.hpp"
#include "commands/status.hpp"
#include "commands/wipe.hpp"
#include "commands/remove.hpp"

// bzip2 might be better than gzip (slower with increased compression ?)
// -> use -j instead of -z in tar commands
// xz might be even better
// -> use -J instead of -z in tar commands

#define MAKE_COMMAND(type, command) return make_shared<type::command##Command>()

#define FIND_COMMAND(command, backend) \
  [backend]() -> shared_ptr<Crails::Command> \
  { \
    if (backend == "tar") \
      MAKE_COMMAND(Tar, command); \
    else if (backend == "bup") \
      MAKE_COMMAND(Bup, command); \
    else \
      throw std::runtime_error("no suitable backup backend found"); \
  }

std::filesystem::path exe_path;

using namespace std;

static string find_backend()
{
  const char* result = getenv("CRAILS_BACKUP_BACKEND");

  if (result == nullptr)
  {
    if (Crails::which("bup").length())
      result = "bup";
    else if (Crails::which("tar").length())
      result = "tar";
  }
  return result;
}

class IndexCommand : public Crails::CommandIndex
{
public:
  IndexCommand()
  {
    const string backend = find_backend();
    add_command("backup",  FIND_COMMAND(Backup, backend));
    add_command("restore", FIND_COMMAND(Restore, backend));
    add_command("list",    FIND_COMMAND(List, backend));
    add_command("add",     []() { return make_shared<AddCommand>(); });
    add_command("remove",  []() { return make_shared<RemoveCommand>(); });
    add_command("status",  []() { return make_shared<StatusCommand>(); });
    add_command("wipe",    []() { return make_shared<WipeCommand>(); });
  }
};

static void find_exe_file(const char* arg0)
{
  error_code error;

  exe_path = filesystem::canonical(arg0, error);
  if (error)
    exe_path = Crails::which(arg0);
  if (!filesystem::exists(exe_path))
    cerr << "could not find executable path from value '" << arg0 << "', tasks related to crontab will fail" << endl;
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
