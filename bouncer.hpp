#pragma once

#include "audio-sink.hpp"
#include "node.hpp"

class Bouncer final : public Node, public AudioSink
{
public:
  float strength = 100.f;

private:
  float offset = 0.f;
  auto ingest(Wav) -> void final;
  auto name() const -> std::string final { return "bouncer"; }
  auto render(Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
};
