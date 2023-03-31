#pragma once
#include "vec2.hpp"
#include <vector>

class Node
{
public:
  virtual ~Node() = default;
  Vec2 loc = {.0f, .0f};
  Vec2 scale = {1.f, 1.f};
  Vec2 pivot = {.0f, .0f};
  float rot = 0.f;
  std::vector<std::reference_wrapper<Node>> nodes;
  auto renderAll() -> void;
  virtual auto renderUi() -> void;

protected:
  virtual auto render() -> void {}

private:
  bool uniformScaling = true;
};
