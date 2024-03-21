#pragma once
#include <crails/cli/command.hpp>
#include <filesystem>

class WipeCommand : public Crails::Command
{
public:
  std::string_view description() const override { return "Clear all backups of a given type"; }

  void options_description(boost::program_options::options_description& desc) const override;
  int run() override;
};
