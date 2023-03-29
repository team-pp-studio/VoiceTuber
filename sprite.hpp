#pragma once
#include "wav-2-visemes.hpp"
#include <SDL_opengl.h>
#include <sdlpp/sdlpp.hpp>
#include <string>

class Sprite
{
public:
  Sprite(const std::string &fileName);
  ~Sprite();
  auto w() const -> int { return w_; };
  auto h() const -> int { return h_; };
  auto render(Viseme) -> void;

private:
  int w_;
  int h_;
  GLuint texture;
};
