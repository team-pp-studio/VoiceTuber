#pragma once
#include "dialog.hpp"
#include "lib.hpp"
#include "texture.hpp"
#include <filesystem>
#include <vector>

class FileOpen final : public Dialog
{
public:
  using Callback = std::move_only_function<void(bool, const std::filesystem::path &)>;
  FileOpen(Lib &, std::string dialogName, Callback);

private:
  auto internalDraw() -> DialogState final;
  auto getSelectedFile() const -> std::filesystem::path;

  std::vector<std::filesystem::path> files;
  std::string selectedFile;
  std::filesystem::path cwd;
  std::shared_ptr<const Texture> upDir;
};
