#pragma once
#include "../backup.hpp"

namespace Tar
{
  class BackupCommand : public BackupCommandBase
  {
  public:
    bool prepare() override;
    bool pack_path(const std::string_view key, const std::filesystem::path&) override;
    bool pack_database(const std::string& url) override;
    bool pack_metadata() override;
    bool store_backup() override;

  private:
    bool gzip_pack();
    std::string add_to_archive_command_prefix() const;
  };
}
