#pragma once
#include "audio-sink.hpp"
#include "node.hpp"

class Bouncer2 final : public Node, public AudioSink
{
public:
#define SER_PROP_LIST \
  SER_PROP(strength); \
  SER_PROP(easing);
  SER_DEF_PROPS()
#undef SER_PROP_LIST

  static constexpr const char *className = "Bouncer2";
  Bouncer2(Lib &, Undo &, class AudioCapture &, std::string name);
  ~Bouncer2() final;

private:
  float strength = 100.f;
  float easing = 50.f;
  float offset = 0.f;
  float a = 0.f;
  std::reference_wrapper<AudioCapture> audioCapture;
  auto ingest(Wav) -> void final;
  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
  auto save(OStrm &) const -> void final;
  auto load(IStrm &) -> void final;
};
