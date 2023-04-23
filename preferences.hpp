#pragma once
#include <string>

class Preferences
{
public:
  Preferences();
  auto save() -> void;
  std::string twitchUser = "mika314";
  std::string twitchKey;
};
