#pragma once

#include "audio-sink.hpp"
#include "node.hpp"

class Bouncer final : public Node, public AudioSink
{
public:
  float strength = 100.f;

  auto renderUi() -> void final;

private:
  float offset = 0.f;
  auto ingest(Wav) -> void final;
  auto render(Node *hovered, Node *selected) -> void final;
};
