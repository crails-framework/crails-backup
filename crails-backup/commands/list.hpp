#pragma once
#include <crails/cli/command.hpp>

class BackupBase;

class ListCommandBase : public Crails::Command
{
public:
  std::string_view description() const override { return "Lists available backup files"; }

  void options_description(boost::program_options::options_description& desc) const override;
  int run() override;
  virtual int list_backups();
  virtual int list_files() = 0;
  int list_files(const BackupBase&);
};
