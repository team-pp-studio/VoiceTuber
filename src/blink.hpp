#pragma once
#include "image-list.hpp"
#include "node.hpp"
#include "sprite-sheet.hpp"
#include "visemes-sink.hpp"
#include <chrono>
#include <filesystem>

template <typename S, typename ClassName>
class Blink final : public Node
{
public:
#define SER_PROP_LIST   \
  SER_PROP(openEyes);   \
  SER_PROP(closedEyes); \
  SER_PROP(blinkEvery); \
  SER_PROP(blinkDuration);

  SER_DEF_PROPS()
#undef SER_PROP_LIST

  Blink(Lib &, Undo &, const std::filesystem::path &);

  constexpr static const char *className = ClassName::v;

private:
  enum class State {
    open,
    closed,
  };

  S sprite;
  int openEyes = 0;
  int closedEyes = 1;
  float blinkEvery = 3.5f;
  float blinkDuration = .25f;
  State state = State::open;
  std::chrono::high_resolution_clock::time_point nextEventTime;

  auto h() const -> float final;
  auto isTransparent(glm::vec2) const -> bool final;
  auto load(IStrm &) -> void final;
  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
  auto save(OStrm &) const -> void final;
  auto w() const -> float final;
  auto do_clone() const -> std::shared_ptr<Node> final;
};

struct SpriteSheetBlinkClassName
{
  constexpr static const char *v = "SpriteSheetBlink";
};

struct ImageListBlinkClassName
{
  constexpr static const char *v = "ImageListBlink";
};

using SpriteSheetBlink = Blink<SpriteSheet, SpriteSheetBlinkClassName>;
using ImageListBlink = Blink<ImageList, ImageListBlinkClassName>;
