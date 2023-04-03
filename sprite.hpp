#pragma once
#include "node.hpp"
#include "wav-2-visemes.hpp"
#include <SDL_opengl.h>
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

  Sprite(const std::string &path);
  ~Sprite() override;
  auto w() const -> float final
  {
    return 1.f * w_ / cols;
  };
  auto h() const -> float final
  {
    return 1.f * h_ / rows;
  };

protected:
  auto render(Node *hovered, Node *selected) -> void override;
  auto renderUi() -> void override;
  auto save(OStrm &) const -> void override;
  auto load(IStrm &) -> void override;

  int cols = 1;
  int rows = 1;
  int frame = 0;
  int numFrames = 1;

private:
  int w_;
  int h_;
  GLuint texture;
};
