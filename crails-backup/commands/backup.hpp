#pragma once
#include <filesystem>
#include <crails/cli/command.hpp>

class BackupCommand : public Crails::Command
{
public:
  ~BackupCommand();

  std::string_view description() const override { return "Perform the backup task, creating a new backup and expiring old backups"; }

  void options_description(boost::program_options::options_description& desc) const override;
  int run() override;

  bool pack_path(const std::string_view key, const std::filesystem::path&);
  bool pack_database(const std::string& url);
  bool pack_metadata();
  bool store_backup();
};
