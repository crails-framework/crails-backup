#include "base.hpp"

using namespace std;

filesystem::path get_backup_folder(const string_view name);

BackupBase::BackupBase(const string& name, const string& id)
  : name(name), path(get_backup_folder(name)), id(id)
{
}
