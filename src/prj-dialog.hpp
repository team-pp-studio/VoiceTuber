#pragma once
#include "dialog.hpp"
#include "lib.hpp"
#include "texture.hpp"
#include <filesystem>
#include <vector>

class PrjDialog final : public Dialog
{
public:
  PrjDialog(Lib &, Callback);

private:
  auto internalDraw() -> DialogState final;

  std::vector<std::filesystem::path> dirs;
  std::string selectedDir;
  std::filesystem::path cwd;
  std::vector<std::filesystem::path> undoStack;
  std::vector<std::filesystem::path> redoStack;
  std::shared_ptr<const Texture> upDir;
  std::shared_ptr<const Texture> bckDir;
  std::shared_ptr<const Texture> fwdDir;
  std::shared_ptr<const Texture> splash;
  std::shared_ptr<const Texture> donate;
  std::shared_ptr<const Texture> github;
};
