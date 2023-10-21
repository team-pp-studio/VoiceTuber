#pragma once
#include "dialog.hpp"
#include <string>

class InputDialog final : public Dialog
{
public:
  using Callback = std::move_only_function<void(bool, const std::string &)>;
  InputDialog(std::string title, std::string init, Callback cb);

private:
  std::string input;

  auto internalDraw() -> DialogState final;
};
