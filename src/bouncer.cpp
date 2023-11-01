#include "bouncer.hpp"
#include "audio-in.hpp"
#include "ui.hpp"
#include <SDL_opengl.h>
#include <limits>
#include <spdlog/spdlog.h>

Bouncer::Bouncer(Lib &lib, Undo &aUndo, class AudioIn &audioIn)
  : Node(lib, aUndo, "bouncer"), audioLevel(audioIn)
{
}

auto Bouncer::do_clone() const -> std::shared_ptr<Node>
{
  return std::make_shared<Bouncer>(*this);
}

auto Bouncer::render(float dt, Node *hovered, Node *selected) -> void
{
  zOrder = INT_MIN;
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
  glClear(GL_COLOR_BUFFER_BIT);
  dLoc.y += std::min(1000.f * dt / 250.f, 1.f) * (strength * audioLevel.getLevel() - dLoc.y);
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
