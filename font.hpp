#pragma once
#include "texture.hpp"
#include <SDL_opengl.h>
#include <SDL_ttf.h>
#include <glm/vec3.hpp>
#include <string>
#include <unordered_map>

class Font
{
public:
  Font(const std::string &, int);
  ~Font();
  auto render(glm::vec3, const std::string &) -> void;
  auto getH(const std::string &) const -> int;

private:
  decltype(TTF_OpenFont("", 0)) font;
  mutable std::unordered_map<std::string, Texture> texturesCache;

  auto getTextureFromCache(const std::string &text) const -> Texture &;
};
