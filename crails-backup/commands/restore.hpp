#pragma once
#include <crails/cli/command.hpp>
#include <filesystem>

class BupBackup;

class RestoreCommand : public Crails::Command
{
  std::filesystem::path tmp_dir;
  std::filesystem::path tmp_src_dir;
public:
  ~RestoreCommand();

  std::string_view description() const override { return "Restore a backup file"; }

  void options_description(boost::program_options::options_description& desc) const override;
  int run() override;

  int restore(const std::string_view name, const std::string& id);
  int unpack(const std::string& id);
  void unpack_file(const BupBackup&, const std::string_view symbol, const std::filesystem::path& target);
  void unpack_directory(const BupBackup&, const std::string_view symbol, const std::filesystem::path& target);
  void unpack_database(const BupBackup&, const std::string_view symbol, const std::string_view url);

  std::map<std::string,std::string> read_metadata();
};
