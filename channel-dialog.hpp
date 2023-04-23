#pragma once
#include "dialog.hpp"
#include <string>

class ChannelDialog final : public Dialog
{
public:
  using Cb = std::function<auto(bool, const std::string &)->void>;
  ChannelDialog(std::string initChannel, Cb cb);

private:
  std::string channel;

  auto internalDraw() -> DialogState final;
};
