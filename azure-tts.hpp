#pragma once
#include <string>

class AzureTts
{
public:
  auto say(const std::string &user, const std::string &msg, bool isMe) -> void;
  auto tick() -> void;
};
