#pragma once
#include "lib.hpp"
#include "node.hpp"
#include <SDL_opengl.h>
#include <filesystem>
#include <sdlpp/sdlpp.hpp>
#include <string>

class SpriteSheet
{
public:
#define SER_PROP_LIST         \
  SER_PROP(cols);             \
  SER_PROP(rows);             \
  SER_PROP(depricatedFrame_); \
  SER_PROP(numFrames_);
  SER_DEF_PROPS()
#undef SER_PROP_LIST

  SpriteSheet(Lib &, Undo &, const std::filesystem::path &path);
  auto frame() const -> int;
  auto frame(int) -> void;
  auto h() const -> float;
  auto isTransparent(glm::vec2) const -> bool;
  auto load(IStrm &) -> void;
  auto numFrames() const -> int;
  auto render() -> void;
  auto renderUi() -> void;
  auto save(OStrm &) const -> void;
  auto w() const -> float;

private:
  std::reference_wrapper<Undo> undo;
  int cols = 1;
  int rows = 1;
  int frame_ = 0;
  int numFrames_ = 1;
  int depricatedFrame_ = 0;
  std::shared_ptr<const Texture> texture;
};
