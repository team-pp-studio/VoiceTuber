#pragma once
#include "texture.hpp"
#include <unordered_map>
#include <memory>

class TexLib
{
public:
  auto queryTex(const std::string &) -> std::shared_ptr<const Texture>;

private:
  std::unordered_map<std::string, std::weak_ptr<const Texture>> lib;
};
