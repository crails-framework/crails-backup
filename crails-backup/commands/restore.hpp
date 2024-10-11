#pragma once
#include <crails/cli/command.hpp>
#include <crails/database_url.hpp>
#include <filesystem>
#include <string_view>

typedef bool (*DatabaseRestoreFunction)(const Crails::DatabaseUrl&, const std::filesystem::path&);
typedef std::map<std::string_view, DatabaseRestoreFunction> DatabaseRestoreFunctionMap;

class RestoreCommandBase : public Crails::Command
{
protected:
  std::filesystem::path tmp_dir;
  std::filesystem::path tmp_src_dir;
  static const DatabaseRestoreFunctionMap restore_functions;
public:
  virtual ~RestoreCommandBase();

  std::string_view description() const override { return "Restore a backup file"; }

  void options_description(boost::program_options::options_description& desc) const override;
  int run() override;

  virtual int restore(const std::string_view name, const std::string& id) = 0;

  static void require_parent_path(const std::filesystem::path&);
};
