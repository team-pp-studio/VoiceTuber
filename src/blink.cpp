#include "blink.hpp"
#include "ui.hpp"

template <typename S, typename ClassName>
Blink<S, ClassName>::Blink(Lib &lib, Undo &aUndo, const std::filesystem::path &path)
  : Node(lib, aUndo, [&path]() { return path.filename().string(); }()),
    sprite(lib, aUndo, path),
    nextEventTime(std::chrono::high_resolution_clock::now())
{
}

template <typename S, typename ClassName>
auto Blink<S, ClassName>::do_clone() const -> std::shared_ptr<Node>
{
  return std::make_shared<Blink>(*this);
}

template <typename S, typename ClassName>
auto Blink<S, ClassName>::h() const -> float
{
  return sprite.h();
}

template <typename S, typename ClassName>
auto Blink<S, ClassName>::isTransparent(glm::vec2 v) const -> bool
{
  return sprite.isTransparent(v);
}

template <typename S, typename ClassName>
auto Blink<S, ClassName>::load(IStrm &strm) -> void
{
  ::deser(strm, *this);
  sprite.load(strm);
  Node::load(strm);
}

template <typename S, typename ClassName>
auto Blink<S, ClassName>::render(float dt, Node *hovered, Node *selected) -> void
{
  if (sprite.numFrames() > 0)
    sprite.frame((state == State::open ? openEyes : closedEyes) % sprite.numFrames());
  sprite.render();
  Node::render(dt, hovered, selected);
  const auto now = std::chrono::high_resolution_clock::now();
  if (now > nextEventTime)
  {
    if (state == State::open)
      state = State::closed;
    else
      state = State::open;
    nextEventTime += state == State::open
                       ? std::chrono::microseconds(static_cast<int64_t>(blinkEvery * 1'000'000))
                       : std::chrono::microseconds(static_cast<int64_t>(blinkDuration * 1'000'000));
  }
}

template <typename S, typename ClassName>
auto Blink<S, ClassName>::renderUi() -> void
{
  Node::renderUi();
  sprite.renderUi();
  ImGui::TableNextColumn();
  Ui::textRj("Open Eyes");
  ImGui::TableNextColumn();
  Ui::inputInt(undo, "##OpenEyes", openEyes);
  ImGui::TableNextColumn();
  Ui::textRj("Closed Eyes");
  ImGui::TableNextColumn();
  Ui::inputInt(undo, "##Closed", closedEyes);
  ImGui::TableNextColumn();
  Ui::textRj("Blink Every");
  ImGui::TableNextColumn();
  if (Ui::dragFloat(undo, "sec##BlinkEvery", blinkEvery, .1f, .01f, 120.f, "%.1f"))
    nextEventTime = std::chrono::high_resolution_clock::now();
  ImGui::TableNextColumn();
  Ui::textRj("Duration");
  ImGui::TableNextColumn();
  if (Ui::dragFloat(undo, "sec##BlinkDuratin", blinkDuration, .01f, .01f, 1.f, "%.3f"))
    nextEventTime = std::chrono::high_resolution_clock::now();
}

template <typename S, typename ClassName>
auto Blink<S, ClassName>::save(OStrm &strm) const -> void
{
  ::ser(strm, className);
  ::ser(strm, name);
  ::ser(strm, *this);
  sprite.save(strm);
  Node::save(strm);
}

template <typename S, typename ClassName>
auto Blink<S, ClassName>::w() const -> float
{
  return sprite.w();
}

template class Blink<SpriteSheet, SpriteSheetBlinkClassName>;
template class Blink<ImageList, ImageListBlinkClassName>;
