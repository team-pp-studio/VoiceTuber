#pragma once
#include "dialog.hpp"
#include "lib.hpp"
#include "texture.hpp"
#include <filesystem>
#include <vector>

class FileOpen final : public Dialog
{
public:
  using Cb = std::function<auto(bool, const std::filesystem::path &)->void>;
  FileOpen(Lib &, std::string dialogName, Cb);

private:
  auto internalDraw() -> DialogState final;
  auto getSelectedFile() const -> std::filesystem::path;

  std::vector<std::filesystem::path> files;
  std::string selectedFile;
  decltype(std::filesystem::current_path()) cwd;
  std::shared_ptr<const Texture> upDir;
};
