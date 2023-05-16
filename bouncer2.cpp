#include "bouncer2.hpp"
#include "audio-capture.hpp"
#include "ui.hpp"
#include <SDL_opengl.h>
#include <limits>
#include <log/log.hpp>

Bouncer2::Bouncer2(Lib &lib, class AudioCapture &audioCapture, std::string aName)
  : Node(lib, std::move(aName)), audioCapture(audioCapture)
{
  audioCapture.reg(*this);
}

Bouncer2::~Bouncer2()
{
  audioCapture.get().unreg(*this);
}

auto Bouncer2::ingest(Wav v) -> void
{
  if (v.empty())
    return;
  a = 1.f * v.back() / 0x8000;
}

auto Bouncer2::render(float dt, Node *hovered, Node *selected) -> void
{
  offset += std::min(1000.f * dt / easing, 1.f) * (strength * a - offset);
  loc.y = offset;
  Node::render(dt, hovered, selected);
}

auto Bouncer2::renderUi() -> void
{
  Node::renderUi();
  ImGui::TableNextColumn();
  Ui::textRj("Bounce");
  ImGui::TableNextColumn();
  ImGui::DragFloat("##Bounce",
                   &strength,
                   1.f,
                   0.f,
                   std::numeric_limits<float>::max(),
                   "%.1f",
                   ImGuiSliderFlags_AlwaysClamp);
  ImGui::TableNextColumn();
  Ui::textRj("Easing");
  ImGui::TableNextColumn();
  ImGui::DragFloat("##Easing",
                   &easing,
                   1.f,
                   5.f,
                   std::numeric_limits<float>::max(),
                   "%.1f",
                   ImGuiSliderFlags_AlwaysClamp);
}

auto Bouncer2::save(OStrm &strm) const -> void
{
  ::ser(strm, className);
  ::ser(strm, name);
  ::ser(strm, *this);
  Node::save(strm);
}
auto Bouncer2::load(IStrm &strm) -> void
{
  ::deser(strm, *this);
  Node::load(strm);
}
