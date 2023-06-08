#pragma once
#include "sprite.hpp"
#include "visemes-sink.hpp"
#include <chrono>

class Mouth final : public Sprite, public VisemesSink
{
public:
#define SER_PROP_LIST SER_PROP(viseme2Sprite);
  SER_DEF_PROPS()
#undef SER_PROP_LIST

  Mouth(class Wav2Visemes &, Lib &, Undo &, const std::filesystem::path &);
  ~Mouth() final;

  static constexpr const char *className = "Mouth";

private:
  std::unordered_map<Viseme, int> viseme2Sprite;
  Viseme viseme;
  decltype(std::chrono::high_resolution_clock::now()) freezeTime;
  std::reference_wrapper<Wav2Visemes> wav2Visemes;

  auto ingest(Viseme) -> void final;
  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
  auto save(OStrm &) const -> void final;
  auto load(IStrm &) -> void final;
};
