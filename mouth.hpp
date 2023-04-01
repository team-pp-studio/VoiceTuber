#pragma once
#include "sprite.hpp"
#include <chrono>

class Mouth final : public Sprite
{
public:
  Mouth(const std::string &fileName);
  std::unordered_map<Viseme, int> viseme2Sprite;

  auto setViseme(Viseme) -> void;

private:
  auto render(Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;

  Viseme viseme;
  decltype(std::chrono::high_resolution_clock::now()) freezeTime;
};
