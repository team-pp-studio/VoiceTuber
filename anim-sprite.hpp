#pragma once
#include "sprite.hpp"
#include <chrono>

class AnimSprite final : public Sprite
{
public:
#define SER_PROP_LIST \
  SER_PROP(fps);      \
  SER_PROP(physics);  \
  SER_PROP(end);      \
  SER_PROP(damping);  \
  SER_PROP(force);    \
  SER_PROP(springness);

  SER_DEF_PROPS()
#undef SER_PROP_LIST

  AnimSprite(const std::string &fileName);
  static constexpr const char *className = "AnimSprite";

private:
  float fps = 30.f;
  bool physics = false;
  glm::vec2 end = {0.f, 0.f};
  float force = 200.f;
  float damping = 1.f;
  float springness = 2.f;
  decltype(std::chrono::high_resolution_clock::now()) startTime;
  float animRotV = 0.f;
  glm::vec2 lastProjPivot = {0.f, 0.f};
  glm::vec2 lastProjPivotV = {0.f, 0.f};

  auto load(IStrm &) -> void final;
  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
  auto save(OStrm &) const -> void final;
};
