#pragma once
#include <string>

class AudioOutput
{
public:
  auto updateDevice(const std::string &) -> void;
};
