#pragma once
#include <filesystem>
#include <vector>

class PrjDialog
{
public:
  auto draw() -> bool;

private:
  std::vector<std::filesystem::path> dirs;
  std::string selectedDir;
};
