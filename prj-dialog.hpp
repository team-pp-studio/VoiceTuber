#pragma once
#include "dialog.hpp"
#include "lib.hpp"
#include "texture.hpp"
#include <filesystem>
#include <vector>

class PrjDialog final : public Dialog
{
public:
  PrjDialog(Lib &, Cb);

private:
  auto internalDraw() -> DialogState final;

  std::vector<std::filesystem::path> dirs;
  std::string selectedDir;
  decltype(std::filesystem::current_path()) cwd;
  std::shared_ptr<const Texture> upDir;
};
