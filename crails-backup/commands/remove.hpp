#pragma once
#include <crails/cli/command.hpp>

class RemoveCommand : public Crails::Command
{
public:
  std::string_view description() const override { return "Permanently remove all backuped file and cron task"; }

  void options_description(boost::program_options::options_description& desc) const override;

  int run() override;
};
