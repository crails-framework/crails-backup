#pragma once
#include <crails/cli/command.hpp>

class StatusCommand : public Crails::Command
{
public:
  std::string_view description() const override { return "Displays the schedule of a backup by name, if it exists"; }

  void options_description(boost::program_options::options_description& desc) const override;
  int run() override;
};
