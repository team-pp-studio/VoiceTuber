#pragma once
#include "audio-level.hpp"
#include "node.hpp"

class Bouncer2 final : public Node
{
public:
#define SER_PROP_LIST \
  SER_PROP(strength); \
  SER_PROP(easing);
  SER_DEF_PROPS()
#undef SER_PROP_LIST

  static constexpr const char *className = "Bouncer2";
  Bouncer2(Lib &, Undo &, class AudioIn &, std::string name);

private:
  float strength = 100.f;
  float easing = 50.f;
  AudioLevel audioLevel;

  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
  auto save(OStrm &) const -> void final;
  auto load(IStrm &) -> void final;
};
