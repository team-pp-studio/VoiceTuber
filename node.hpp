#pragma once
#include "vec2.hpp"
#include <vector>
#include <glm/gtc/type_ptr.hpp>

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
  auto screenToLocal(const glm::mat4 &projMat, Vec2) const -> Vec2;

protected:
  virtual auto render() -> void {}

private:
  bool uniformScaling = true;
  glm::mat4 modelViewMat;
};
