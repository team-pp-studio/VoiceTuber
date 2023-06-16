#pragma once
#include "lib.hpp"
#include "node.hpp"
#include "wav-2-visemes.hpp"
#include <SDL_opengl.h>
#include <filesystem>
#include <sdlpp/sdlpp.hpp>
#include <string>
#include <unordered_map>

class Sprite
{
public:
#define SER_PROP_LIST \
  SER_PROP(cols);     \
  SER_PROP(rows);     \
  SER_PROP(frame_);   \
  SER_PROP(numFrames_);
  SER_DEF_PROPS()
#undef SER_PROP_LIST

  Sprite(Lib &, Undo &, const std::filesystem::path &path);
  auto h() const -> float;
  auto isTransparent(glm::vec2) const -> bool;
  auto load(IStrm &) -> void;
  auto render() -> void;
  auto renderUi() -> void;
  auto save(OStrm &) const -> void;
  auto w() const -> float;
  auto frame(int) -> void;
  auto frame() const -> int;
  auto numFrames() const -> int;

private:
  std::reference_wrapper<Undo> undo;
  int cols = 1;
  int rows = 1;
  int frame_ = 0;
  int numFrames_ = 1;
  std::shared_ptr<const Texture> texture;
};
