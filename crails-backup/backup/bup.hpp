#pragma once
#include <filesystem>
#include <string>
#include <map>
#include "base.hpp"
#include "../list_mode.hpp"

namespace Crails { class ExecutableCommand; }

class BupBackup : public BackupBase
{
public:
  BupBackup(const std::string& name, const std::string& id = "");

  bool        wipe() const override;
  Metadata    read_metadata() const override;
  BackupList  list() const override;

  std::string path_prefix() const;
  static void append_server_options(Crails::ExecutableCommand&);
};
