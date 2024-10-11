#include "list.hpp"
#include "../../backup/tar.hpp"

using namespace std;
using namespace Tar;

int ListCommand::list_files()
{
  return ListCommandBase::list_files(
    TarBackup(options["name"].as<string>())
  );
}
