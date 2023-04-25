#pragma once
#include "dialog.hpp"
#include <string>

class MessageDialog final : public Dialog
{
public:
  MessageDialog(std::string title, std::string msg);

private:
  std::string msg;

  auto internalDraw() -> DialogState final;
};
