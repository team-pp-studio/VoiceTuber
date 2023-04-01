#pragma once
#include "sprite.hpp"

class Mouth final : public Sprite
{
public:
  Mouth(const std::string &fileName);
  Viseme viseme;
  std::unordered_map<Viseme, int> viseme2Sprite;

private:
  auto render(Node *hovered, Node *selected) -> void final;
};
