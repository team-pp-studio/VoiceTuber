#pragma once
#include "dialog.hpp"
#include <string>

class InputDialog final : public Dialog
{
public:
  using Cb = std::function<auto(bool, const std::string &)->void>;
  InputDialog(std::string title, std::string init, Cb cb);

private:
  std::string input;

  auto internalDraw() -> DialogState final;
};
