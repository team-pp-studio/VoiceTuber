#pragma once
#include "sprite.hpp"
#include "visemes-sink.hpp"
#include <chrono>

class Mouth final : public Sprite, public VisemesSink
{
public:
  Mouth(class Wav2Visemes &, const std::string &fileName);
  ~Mouth();
  std::unordered_map<Viseme, int> viseme2Sprite;

private:
  auto ingest(Viseme) -> void final;
  auto name() const -> std::string final;
  auto render(Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;

  Viseme viseme;
  decltype(std::chrono::high_resolution_clock::now()) freezeTime;
  std::reference_wrapper<Wav2Visemes> wav2Visemes;
};
