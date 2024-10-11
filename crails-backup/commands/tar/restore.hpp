#pragma once
#include "../restore.hpp"

class TarBackup;

namespace Tar
{
  class RestoreCommand : public RestoreCommandBase
  {
  public:
    int restore(const std::string_view name, const std::string& id) override;
    int unpack(const TarBackup&);
    void unpack_file(const TarBackup&, const std::string_view symbol, const std::filesystem::path& target);
    void unpack_directory(const TarBackup&, const std::string_view symbol, const std::filesystem::path& target);
    void unpack_database(const TarBackup&, const std::string_view symbol, const std::string_view url);

    std::map<std::string,std::string> read_metadata();
  };
}
