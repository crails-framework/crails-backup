#include "base.hpp"
#include <algorithm>
#include <functional>

using namespace std;

filesystem::path get_backup_folder(const string_view name);

BackupBase::BackupBase(const string& name, const string& id)
  : name(name), path(get_backup_folder(name)), id(id)
{
}

static bool compare_backup_list_value(const BackupList::value_type& a, const BackupList::value_type& b)
{
  return a.second < b.second;
}

void BackupSortFunctor::sort(BackupList& list)
{
  std::sort(list.begin(), list.end(), compare_backup_list_value);
}
