#pragma once

#include "audio-level.hpp"
#include "node.hpp"
#include <imgui.h>

class Bouncer final : public Node
{
public:
#define SER_PROP_LIST \
  SER_PROP(strength); \
  SER_PROP(clearColor);
  SER_DEF_PROPS()
#undef SER_PROP_LIST

  static constexpr const char *className = "Bouncer";
  Bouncer(Lib &, Undo &, class AudioIn &);

private:
  float strength = 100.f;
  ImVec4 clearColor = ImVec4(123.f / 256.f, 164.f / 256.f, 119.f / 256.f, 1.00f);
  AudioLevel audioLevel;
  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
  auto save(OStrm &) const -> void final;
  auto load(IStrm &) -> void final;
};
