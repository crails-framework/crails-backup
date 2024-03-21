#include "wipe.hpp"

using namespace boost;
using namespace std;

filesystem::path get_backup_folder(const string_view name);

void WipeCommand::options_description(boost::program_options::options_description& desc) const
{
  desc.add_options()
    ("name,n", program_options::value<string>(), "backup name");
}

int WipeCommand::run()
{
  if (options.count("name"))
  {
    string name = options["name"].as<string>();
    error_code ec;

    filesystem::remove_all(get_backup_folder(name), ec);
    return !ec ? 0 : 1;
  }
  return -1;
}
