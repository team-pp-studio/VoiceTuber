#pragma once

#include "viseme.hpp"

class VisemesSink
{
public:
  virtual ~VisemesSink() = default;
  virtual auto ingest(Viseme) -> void = 0;
};
