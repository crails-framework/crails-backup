#include "list.hpp"
#include "../../backup/bup.hpp"

using namespace std;

int Bup::ListCommand::list_files()
{
  return ListCommandBase::list_files(
    BupBackup(options["name"].as<string>())
  );
}
