#include <crails/crontab.hpp>
#include "remove.hpp"

using namespace boost;
using namespace std;

void RemoveCommand::options_description(boost::program_options::options_description& desc) const
{
  desc.add_options()
    ("name,n", program_options::value<string>(), "backup name");
}

int RemoveCommand::run()
{
  if (options.count("name"))
  {
    Crails::Crontab crontab;

    crontab.load();
    crontab.remove_task(options["name"].as<string>());
    return crontab.save() ? 0 : 1;
  }
  return -1;
}
