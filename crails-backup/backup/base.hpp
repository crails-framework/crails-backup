#pragma once
#include <chrono>
#include <filesystem>
#include <vector>
#include <string>
#include "../metadata.hpp"

typedef std::vector<std::pair<std::string, std::chrono::system_clock::time_point> > BackupList;

namespace BackupSortFunctor
{
  void sort(BackupList& list);
};

struct BackupBase
{
  BackupBase(const std::string& name, const std::string& id = "");

  virtual bool          wipe() const = 0;
  virtual Metadata      read_metadata() const = 0;
  virtual BackupList    list() const = 0;

  void set_backup_id(const std::string& value) { id = value; }

  std::string           name;
  std::filesystem::path path;
  std::string           id;
};
