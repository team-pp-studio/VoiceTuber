#pragma once
#include "dialog.hpp"
#include <filesystem>
#include <vector>

class PrjDialog final : public Dialog
{
public:
  PrjDialog(Cb cb);

private:
  auto internalDraw() -> DialogState final;

  std::vector<std::filesystem::path> dirs;
  std::string selectedDir;
};
