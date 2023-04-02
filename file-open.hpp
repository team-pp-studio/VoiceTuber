#pragma once
#include <filesystem>
#include <vector>

class FileOpen
{
private:
  std::vector<std::filesystem::path> files;
  std::filesystem::path selectedFile;

public:
  FileOpen(const char *dialogName) : dialogName(dialogName) {}
  auto draw() -> bool;
  auto getSelectedFile() const -> std::filesystem::path;

  const char *dialogName;
};
