#pragma once
#include "sprite.hpp"
#include <chrono>

class AnimSprite : public Sprite
{
public:
#define SER_PROP_LIST SER_PROP(fps);
  SER_DEF_PROPS()
#undef SER_PROP_LIST

  AnimSprite(const std::string &fileName);
  static constexpr const char *className = "AnimSprite";

protected:
  auto render(Node *hovered, Node *selected) -> void override;
  auto renderUi() -> void override;

private:
  float fps = 30.f;
  decltype(std::chrono::high_resolution_clock::now()) startTime;

  auto save(OStrm &) const -> void final;
  auto load(IStrm &) -> void final;
};
