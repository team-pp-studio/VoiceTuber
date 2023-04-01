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
  Sprite(const std::string &fileName);
  ~Sprite() override;
  auto w() const -> float final { return 1.f * w_ / cols; };
  auto h() const -> float final { return 1.f * h_ / rows; };

  int cols = 1;
  int rows = 1;
  int frame = 0;
  int numFrames = 1;

protected:
  auto render(Node *hovered, Node *selected) -> void override;
  auto renderUi() -> void override;

private:
  int w_;
  int h_;
  GLuint texture;
};
