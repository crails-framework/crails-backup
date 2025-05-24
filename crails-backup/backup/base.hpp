#pragma once
#include <chrono>
#include <filesystem>
#include <map>
#include <string>
#include "../metadata.hpp"

struct BackupSortFunctor
{
  bool operator()(const std::string& a, const std::string& b) const
  {
    return std::stoi(a) < std::stoi(b);
  }
};

typedef std::map<std::string, std::chrono::system_clock::time_point, BackupSortFunctor> BackupList;

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
