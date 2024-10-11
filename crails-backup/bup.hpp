#pragma once
#include <filesystem>
#include <string>
#include <map>
#include "list_mode.hpp"

typedef std::map<std::string,std::string> Metadata;

class BupBackup
{
public:
  BupBackup(const std::string& name, const std::string& id = "");

  bool init() const;
  bool index(const std::filesystem::path&) const;
  bool save(const std::filesystem::path&) const;
  bool restore(const std::filesystem::path&, const std::string_view key) const;
  bool list(ListMode) const;
  Metadata    read_metadata() const;
  std::string path_prefix() const;

  void set_backup_id(const std::string& value) { id = value; }

private:
  std::string           name;
  std::filesystem::path path;
  std::string           id;
};
