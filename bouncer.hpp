#pragma once

#include "audio-sink.hpp"
#include "node.hpp"
#include <imgui/imgui.h>

class Bouncer final : public Node, public AudioSink
{
public:
#define SER_PROP_LIST \
  SER_PROP(strength); \
  SER_PROP(clearColor);
  SER_DEF_PROPS()
#undef SER_PROP_LIST

  static constexpr const char *className = "Bouncer";
  Bouncer(class AudioCapture &);
  ~Bouncer();

private:
  float strength = 100.f;
  ImVec4 clearColor = ImVec4(123.f / 256.f, 164.f / 256.f, 119.f / 256.f, 1.00f);
  float offset = 0.f;
  std::reference_wrapper<AudioCapture> audioCapture;
  auto ingest(Wav) -> void final;
  auto render(Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
  auto save(OStrm &) const -> void final;
  auto load(IStrm &) -> void final;
};
