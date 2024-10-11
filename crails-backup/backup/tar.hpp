#pragma once
#include <filesystem>
#include <chrono>
#include <map>
#include "base.hpp"

class TarBackup : public BackupBase
{
public:
  TarBackup(const std::string& name, const std::string& id = "");

  Metadata   read_metadata() const override;
  bool       wipe() const override;
  BackupList list() const override;

  std::filesystem::path archive_path() const;
};
