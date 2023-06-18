#include "version.hpp"

auto appVersion() -> const char *
{
  return "0.0.3";
}

auto saveVersion() -> uint32_t
{
  return 2;
}
