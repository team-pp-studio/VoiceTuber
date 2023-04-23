#pragma once
#include "dialog.hpp"
#include <string>

class ChannelDialog final : public Dialog
{
public:
  using Cb = std::function<auto(bool, const std::string &)->void>;
  ChannelDialog(std::string initChannel, Cb cb);
  auto draw() -> bool final;

private:
  std::string channel;
};
