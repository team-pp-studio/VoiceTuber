#pragma once

#include "node.hpp"
#include <imgui/imgui.h>

class Root final : public Node
{
public:
#define SER_PROP_LIST SER_PROP(clearColor);
  SER_DEF_PROPS()
#undef SER_PROP_LIST

  static constexpr const char *className = "Root";
  Root(class Lib &, Undo &);
  ~Root() final;

private:
  ImVec4 clearColor = ImVec4(123.f / 256.f, 164.f / 256.f, 119.f / 256.f, 1.00f);
  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
  auto save(OStrm &) const -> void final;
  auto load(IStrm &) -> void final;
};
