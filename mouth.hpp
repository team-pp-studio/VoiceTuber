#pragma once
#include "image-list.hpp"
#include "node.hpp"
#include "sprite-sheet.hpp"
#include "visemes-sink.hpp"
#include <chrono>
#include <filesystem>

template <typename S, typename ClassName>
class Mouth final : public Node, public VisemesSink
{
public:
#define SER_PROP_LIST SER_PROP(viseme2Sprite);
  SER_DEF_PROPS()
#undef SER_PROP_LIST

  Mouth(class Wav2Visemes &, Lib &, Undo &, const std::filesystem::path &);
  ~Mouth() final;

  constexpr static const char *className = ClassName::v;

private:
  S sprite;
  std::map<Viseme, int> viseme2Sprite;
  Viseme viseme = Viseme{};
  decltype(std::chrono::high_resolution_clock::now()) freezeTime;
  std::reference_wrapper<Wav2Visemes> wav2Visemes;

  auto h() const -> float final;
  auto ingest(Viseme) -> void final;
  auto isTransparent(glm::vec2) const -> bool final;
  auto load(IStrm &) -> void final;
  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
  auto save(OStrm &) const -> void final;
  auto w() const -> float final;
};

struct SpriteSheetMouthClassName
{
  constexpr static const char *v = "Mouth";
};

struct ImageListMouthClassName
{
  constexpr static const char *v = "ImageListMouth";
};

using SpriteSheetMouth = Mouth<SpriteSheet, SpriteSheetMouthClassName>;
using ImageListMouth = Mouth<ImageList, ImageListMouthClassName>;
