#pragma once
#include "sprite.hpp"
#include <chrono>

class AnimSprite : public Sprite
{
public:
  AnimSprite(const std::string &fileName);
  float fps = 30.f;

protected:
  auto render(Node *hovered, Node *selected) -> void override;
  auto renderUi() -> void override;

private:
  decltype(std::chrono::high_resolution_clock::now()) startTime;
};
