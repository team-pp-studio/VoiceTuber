#pragma once
#include "vec2.hpp"
#include <glm/gtc/type_ptr.hpp>
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
  auto renderAll(Node *hovered, Node *selected) -> void;
  auto nodeUnder(const glm::mat4 &projMap, Vec2) -> Node *;
  virtual auto h() const -> float { return 1.f; }
  virtual auto renderUi() -> void;
  virtual auto w() const -> float { return 1.f; }

protected:
  virtual auto render(Node *hovered, Node *selected) -> void;

private:
  auto screenToLocal(const glm::mat4 &projMat, Vec2) const -> Vec2;
  bool uniformScaling = true;
  glm::mat4 modelViewMat;
};
