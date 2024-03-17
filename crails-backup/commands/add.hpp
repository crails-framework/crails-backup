#pragma once
#include <crails/cli/command.hpp>

class AddCommand : public Crails::Command
{
public:
  std::string_view description() const override { return "Configure a new backup and its corresponding cron task"; }

  void options_description(boost::program_options::options_description& desc) const override;
  int run() override;

private:
  std::string make_command() const;
  int update_crontab() const;
};
