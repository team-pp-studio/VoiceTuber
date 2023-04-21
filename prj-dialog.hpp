#pragma once
#include "dialog.hpp"
#include <filesystem>
#include <vector>

class PrjDialog final : public Dialog
{
public:
  PrjDialog(Cb cb);
  auto draw() -> bool final;

private:
  std::vector<std::filesystem::path> dirs;
  std::string selectedDir;
};
