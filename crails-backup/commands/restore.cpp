#include "restore.hpp"
#include <crails/cli/with_path.hpp>
#include <filesystem>
#include <cstdlib>
#include <iostream>
#include <fstream>

using namespace std;

bool restore_mysql(const Crails::DatabaseUrl&, const filesystem::path&);
bool restore_postgres(const Crails::DatabaseUrl&, const filesystem::path&);
bool restore_mongodb(const Crails::DatabaseUrl&, const filesystem::path&);

const DatabaseRestoreFunctionMap RestoreCommandBase::restore_functions = {
  {"mysql",    &restore_mysql},
  {"postgres", &restore_postgres},
  {"mongodb",  &restore_mongodb}
};

void RestoreCommandBase::require_parent_path(const filesystem::path& path)
{
  filesystem::path parent_path = path.parent_path();

  if (!filesystem::exists(parent_path))
  {
    filesystem::create_directories(parent_path);
  }
  else if (!filesystem::is_directory(parent_path))
  {
    throw runtime_error("path " + path.string() + " was expected to be a directory, but it isn't.");
  }
}

RestoreCommandBase::~RestoreCommandBase()
{
  if (filesystem::is_directory(tmp_dir))
    filesystem::remove_all(tmp_dir);
}

void RestoreCommandBase::options_description(boost::program_options::options_description& desc) const
{
  desc.add_options()
    ("name,n", boost::program_options::value<std::string>(), "backup name")
    ("id",     boost::program_options::value<std::string>(), "backup id");
}

int RestoreCommandBase::run()
{
  if (options.count("name") && options.count("id"))
  {
    const string name = options["name"].as<string>();
    const string id = options["id"].as<string>();

    tmp_dir = filesystem::temp_directory_path() / ("restore-" + name);
    tmp_src_dir = filesystem::temp_directory_path() / ("backup-" + name);
    return restore(name, id);
  }
  else
    cerr << "missing name or id option" << endl;
  return -1;
}
