#include "mouth.hpp"

Mouth::Mouth(const std::string &fileName) : Sprite(fileName) {}

auto Mouth::render(Node *hovered, Node *selected) -> void
{
  frame = viseme2Sprite[viseme];
  Sprite::render(hovered, selected);
}
