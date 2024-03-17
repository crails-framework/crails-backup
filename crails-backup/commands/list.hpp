#pragma once
#include <crails/cli/command.hpp>

class ListCommand : public Crails::Command
{
public:
  std::string_view description() const override { return "Lists available backup files"; }

  void options_description(boost::program_options::options_description& desc) const override;
  int run() override;
  int list_backups();
  int list_files();
};
