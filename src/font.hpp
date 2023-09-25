#pragma once
#include "texture.hpp"
#include <SDL_opengl.h>
#include <SDL_ttf.h>
#include <filesystem>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <list>
#include <string>
#include <unordered_map>

class Font
{
public:
  Font(std::filesystem::path, int);
  ~Font();
  auto render(glm::vec2, const std::string &) -> void;
  auto getSize(const std::string &) const -> glm::vec2;
  auto file() const -> const std::filesystem::path &;
  auto ptsize() const -> int;

private:
  std::filesystem::path file_;
  int ptsize_;
  decltype(TTF_OpenFont("", 0)) font;
  mutable std::unordered_map<std::string, std::pair<Texture, std::list<std::string>::iterator>>
    texturesCache;
  mutable std::list<std::string> cacheAge;

  auto getTextureFromCache(const std::string &text) const -> Texture &;
};
