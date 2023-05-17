#pragma once
#include "lib.hpp"
#include "node.hpp"
#include "wav-2-visemes.hpp"
#include <SDL_opengl.h>
#include <filesystem>
#include <sdlpp/sdlpp.hpp>
#include <string>
#include <unordered_map>

class Sprite : public Node
{
public:
#define SER_PROP_LIST \
  SER_PROP(cols);     \
  SER_PROP(rows);     \
  SER_PROP(frame);    \
  SER_PROP(numFrames);
  SER_DEF_PROPS()
#undef SER_PROP_LIST

  Sprite(Lib &, Undo &, const std::filesystem::path &path);
  auto w() const -> float final;
  auto h() const -> float final;
  auto isTransparent(glm::vec2) const -> bool final;

protected:
  auto render(float dt, Node *hovered, Node *selected) -> void override;
  auto renderUi() -> void override;
  auto save(OStrm &) const -> void override;
  auto load(IStrm &) -> void override;

  int cols = 1;
  int rows = 1;
  int frame = 0;
  int numFrames = 1;

private:
  std::shared_ptr<const Texture> texture;
};
