#pragma once
#include <string>

class Preferences
{
public:
  Preferences();
  auto save() -> void;

  constexpr static const char *DefaultAudio = "Default";

  std::string twitchUser = "mika314";
  std::string twitchKey;
  std::string outputAudio = DefaultAudio;
  std::string inputAudio = DefaultAudio;
};
