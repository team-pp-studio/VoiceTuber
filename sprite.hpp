#pragma once
#include "node.hpp"
#include "wav-2-visemes.hpp"
#include <SDL_opengl.h>
#include <sdlpp/sdlpp.hpp>
#include <string>

class Sprite : public Node
{
public:
  Sprite(const std::string &fileName);
  ~Sprite();
  auto w() const -> int { return w_; };
  auto h() const -> int { return h_; };

  int cols = 1;
  int rows = 1;
  Viseme viseme;
  auto renderUi() -> void final;

private:
  int w_;
  int h_;
  GLuint texture;
  auto render() -> void final;
};
