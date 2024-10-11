#pragma once
#include "../backup.hpp"

namespace Bup
{
  class BackupCommand : public BackupCommandBase
  {
  public:
    bool store_backup() override;
  };
}
