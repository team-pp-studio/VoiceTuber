#pragma once
#include "node.hpp"
#include "sprite.hpp"
#include <chrono>

class AnimSprite : public Node
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

  AnimSprite(Lib &, Undo &, const std::filesystem::path &);
  static constexpr const char *className = "AnimSprite";

protected:
  auto render(float dt, Node *hovered, Node *selected) -> void override;
  auto save(OStrm &) const -> void override;
  auto load(IStrm &) -> void override;
  auto renderUi() -> void override;

protected:
  Sprite sprite;

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
  std::shared_ptr<const Texture> arrowN;
  std::shared_ptr<const Texture> arrowNE;
  std::shared_ptr<const Texture> arrowE;
  std::shared_ptr<const Texture> arrowSE;
  std::shared_ptr<const Texture> arrowS;
  std::shared_ptr<const Texture> arrowSW;
  std::shared_ptr<const Texture> arrowW;
  std::shared_ptr<const Texture> arrowNW;
  std::shared_ptr<const Texture> center;

  auto h() const -> float final;
  auto isTransparent(glm::vec2) const -> bool final;
  auto w() const -> float final;
};
