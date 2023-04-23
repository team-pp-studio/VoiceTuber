#pragma once
#include "dialog.hpp"
#include <filesystem>
#include <vector>

class FileOpen final : public Dialog
{
public:
  using Cb = std::function<auto(bool, const std::filesystem::path &)->void>;
  FileOpen(std::string dialogName, Cb);
  auto draw() -> bool final;

private:
  auto getSelectedFile() const -> std::filesystem::path;

  std::vector<std::filesystem::path> files;
  std::filesystem::path selectedFile;
  decltype(std::filesystem::current_path()) cwd;
};
