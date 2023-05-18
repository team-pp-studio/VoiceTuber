#include "bouncer.hpp"
#include "audio-capture.hpp"
#include "ui.hpp"
#include <SDL_opengl.h>
#include <limits>
#include <log/log.hpp>

Bouncer::Bouncer(Lib &lib, Undo &aUndo, class AudioCapture &audioCapture)
  : Node(lib, aUndo, "bouncer"), audioCapture(audioCapture)
{
  audioCapture.reg(*this);
}

Bouncer::~Bouncer()
{
  audioCapture.get().unreg(*this);
}

auto Bouncer::ingest(Wav v) -> void
{
  if (v.empty())
    return;
  offset = strength * v.back() / 0x8000;
}

auto Bouncer::render(float dt, Node *hovered, Node *selected) -> void
{
  zOrder = INT_MIN;
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
  glClear(GL_COLOR_BUFFER_BIT);
  loc.y = offset;
  Node::render(dt, hovered, selected);
}

auto Bouncer::renderUi() -> void
{
  Node::renderUi();
  ImGui::TableNextColumn();
  Ui::textRj("BG color");
  ImGui::TableNextColumn();
  ImGui::ColorEdit4(
    "##BG color", (float *)&clearColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
  ImGui::TableNextColumn();
  Ui::textRj("Bounce");
  ImGui::TableNextColumn();
  Ui::dragFloat(undo,
                "##Bounce",
                strength,
                1.f,
                0.f,
                std::numeric_limits<float>::max(),
                "%.1f",
                ImGuiSliderFlags_AlwaysClamp);
}

auto Bouncer::save(OStrm &strm) const -> void
{
  ::ser(strm, className);
  ::ser(strm, name);
  ::ser(strm, *this);
  Node::save(strm);
}
auto Bouncer::load(IStrm &strm) -> void
{
  ::deser(strm, *this);
  Node::load(strm);
}
