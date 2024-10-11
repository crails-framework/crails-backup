#pragma once
#include "../list.hpp"

namespace Tar
{
  class ListCommand : public ListCommandBase
  {
  public:
    int list_files() override;
  };
}
