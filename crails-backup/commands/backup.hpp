#pragma once
#include <filesystem>
#include <string_view>
#include <crails/cli/command.hpp>
#include <crails/database_url.hpp>

typedef bool (*DatabaseDumpFunction)(const Crails::DatabaseUrl&, const std::filesystem::path&);
typedef std::map<std::string_view, DatabaseDumpFunction> DatabaseBackupFunctionMap;

class BackupCommandBase : public Crails::Command
{
protected:
  std::filesystem::path tmp_dir;
  std::map<std::string, std::string> name_to_path_map;
  static const DatabaseBackupFunctionMap dump_functions;
public:
  virtual ~BackupCommandBase();

  std::string_view description() const override { return "Perform the backup task, creating a new backup and expiring old backups"; }

  void options_description(boost::program_options::options_description& desc) const override;
  int run() override;

  virtual bool prepare();
  virtual bool pack_path(const std::string_view key, const std::filesystem::path&);
  virtual bool pack_database(const std::string& url);
  virtual bool pack_metadata();
  virtual bool store_backup() = 0;
};
