#include <crails/crontab.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "add.hpp"

extern std::filesystem::path exe_path;

using namespace boost;
using namespace std;

void AddCommand::options_description(boost::program_options::options_description& desc) const
{
  desc.add_options()
    ("name,n",      program_options::value<string>(),         "backup name")
    ("databases,d", program_options::value<vector<string>>(), "list of database urls")
    ("files,f",     program_options::value<vector<string>>(), "list of file and directories")
    ("schedule,s",  program_options::value<string>(),         "cron schedule (such as `* 0 1 * *`)");
}

int AddCommand::run()
{
  return options.count("name") && options.count("schedule")
    ? update_crontab()
    : -1;
}

string AddCommand::make_command() const
{
  string name = options["name"].as<string>();
  stringstream stream;

  stream << exe_path << " backup -n " << quoted(name);
  if (options.count("databases"))
  {
    stream << " -d";
    for (const auto& url : options["databases"].as<vector<string>>())
      stream << ' ' << quoted(url);
  }
  if (options.count("files"))
  {
    stream << " -f";
    for (const auto& path : options["files"].as<vector<string>>())
      stream << ' ' << quoted(path);
  }
  return stream.str();
}

int AddCommand::update_crontab() const
{
  Crails::Crontab crontab;

  crontab.load();
  crontab.set_variable("LD_LIBRARY_PATH", "/usr/local/lib");
  crontab.set_task(
    options["name"].as<string>(),
    options["schedule"].as<string>(),
    make_command()
  );
  //return crontab.save() ? 0 : 1;

  // crontab.save does not seem to work. this does:
  stringstream command;
  string crontab_txt = crontab.save_to_string();
  filesystem::path tmp_path = filesystem::temp_directory_path() / "crontab_txt";

  ofstream stream(tmp_path);
  stream << crontab_txt;
  stream.close();

  command << "cat " << tmp_path << " | crontab";
  int result = system(command.str().c_str());
  filesystem::remove(tmp_path);
  return result;
}

