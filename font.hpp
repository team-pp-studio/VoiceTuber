#pragma once
#include "texture.hpp"
#include <SDL_opengl.h>
#include <SDL_ttf.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <string>
#include <unordered_map>

class Font
{
public:
  Font(std::string, int);
  ~Font();
  auto render(glm::vec3, const std::string &) -> void;
  auto getSize(const std::string &) const -> glm::vec2;
  auto file() const -> const std::string &;
  auto ptsize() const -> int;

private:
  std::string file_;
  int ptsize_;
  decltype(TTF_OpenFont("", 0)) font;
  mutable std::unordered_map<std::string, Texture> texturesCache;

  auto getTextureFromCache(const std::string &text) const -> Texture &;
};
