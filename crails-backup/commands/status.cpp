#include <crails/crontab.hpp>
#include <iostream>
#include "status.hpp"

using namespace std;

void StatusCommand::options_description(boost::program_options::options_description& desc) const
{
  desc.add_options()
    ("name,n", boost::program_options::value<string>(), "backup name");
}

int StatusCommand::run()
{
  if (options.count("name"))
  {
    Crails::Crontab crontab;
    optional<Crails::Crontab::Task> task;

    crontab.load();
    task = crontab.get_task(options["name"].as<string>());
    if (task)
    {
      cout << task->schedule << endl;
      return 0;
    }
    return 1;
  }
  return -1;
}
