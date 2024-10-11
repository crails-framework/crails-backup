#pragma once
#include "../list.hpp"

namespace Bup
{
  class ListCommand : public ListCommandBase
  {
  public:
    int list_files() override;
  };
}
