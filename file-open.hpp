#pragma once
#include "dialog.hpp"
#include <filesystem>
#include <vector>

class FileOpen final : public Dialog
{
public:
  using Cb = std::function<auto(const std::filesystem::path &)->void>;
  FileOpen(const char *dialogName, Cb);
  auto draw() -> bool final;

private:
  auto getSelectedFile() const -> std::filesystem::path;

  std::vector<std::filesystem::path> files;
  std::filesystem::path selectedFile;
  const char *dialogName;
  decltype(std::filesystem::current_path()) cwd;
};
