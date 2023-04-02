#pragma once
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <string>
#include <vector>

class Node
{
public:
  virtual ~Node() = default;
  glm::vec2 loc = {.0f, .0f};
  glm::vec2 scale = {1.f, 1.f};
  glm::vec2 pivot = {.0f, .0f};
  float rot = 0.f;
  auto addChild(Node &) -> void;
  auto moveDown() -> void;
  auto moveUp() -> void;
  auto nodeUnder(const glm::mat4 &projMat, glm::vec2) -> Node *;
  auto nodes() const -> const std::vector<std::reference_wrapper<Node>> &;
  auto parent() -> Node * { return parent_; };
  auto parentWithBellow() -> void;
  auto renderAll(Node *hovered, Node *selected) -> void;
  auto unparent() -> void;
  auto updateLoc(const glm::mat4 &projMat,
                 glm::vec2 initLoc,
                 glm::vec2 startScreenLoc,
                 glm::vec2 endScreenLoc) -> void;
  auto updateRot(const glm::mat4 &projMat,
                 float initRot,
                 glm::vec2 startScreenLoc,
                 glm::vec2 endScreenLoc) -> void;
  auto updateScale(const glm::mat4 &projMat,
                   glm::vec2 initScale,
                   glm::vec2 startScreenLoc,
                   glm::vec2 endScreenLoc) -> void;
  virtual auto h() const -> float { return 1.f; }
  virtual auto name() const -> std::string { return "node"; }
  virtual auto renderUi() -> void;
  virtual auto w() const -> float { return 1.f; }

protected:
  virtual auto render(Node *hovered, Node *selected) -> void;

private:
  auto screenToLocal(const glm::mat4 &projMat, glm::vec2) const -> glm::vec2;
  std::vector<std::reference_wrapper<Node>> nodes_;
  bool uniformScaling = true;
  glm::mat4 modelViewMat;
  Node *parent_ = nullptr;
};
