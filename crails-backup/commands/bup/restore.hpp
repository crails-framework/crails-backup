#pragma once
#include "../restore.hpp"

class BupBackup;

namespace Bup
{
  class RestoreCommand : public RestoreCommandBase
  {
  public:
    int restore(const std::string_view name, const std::string& id) override;
    int unpack(const std::string& id);
    void unpack_file(const BupBackup&, const std::string_view symbol, const std::filesystem::path& target);
    void unpack_directory(const BupBackup&, const std::string_view symbol, const std::filesystem::path& target);
    void unpack_database(const BupBackup&, const std::string_view symbol, const std::string_view url);
  };
}
