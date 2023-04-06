#pragma once
#include "texture.hpp"
#include <unordered_map>

class TexLib
{
public:
  auto queryTex(const std::string &) -> const Texture &;

private:
  std::unordered_map<std::string, const Texture> lib;
};
