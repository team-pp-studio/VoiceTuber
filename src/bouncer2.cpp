#include "bouncer2.hpp"
#include "audio-in.hpp"
#include "ui.hpp"
#include <SDL_opengl.h>
#include <limits>
#include <spdlog/spdlog.h>

Bouncer2::Bouncer2(Lib &lib, Undo &aUndo, class AudioIn &audioIn, std::string aName)
  : Node(lib, aUndo, std::move(aName)), audioLevel(audioIn)
{
}

auto Bouncer2::do_clone() const -> std::shared_ptr<Node>
{
  return std::make_shared<Bouncer2>(*this);
}

auto Bouncer2::render(float dt, Node *hovered, Node *selected) -> void
{
  dLoc.y += std::min(1000.f * dt / easing, 1.f) * (strength * audioLevel.getLevel() - dLoc.y);
  Node::render(dt, hovered, selected);
}

auto Bouncer2::renderUi() -> void
{
  Node::renderUi();
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
  ImGui::TableNextColumn();
  Ui::textRj("Easing");
  ImGui::TableNextColumn();
  Ui::dragFloat(undo,
                "##Easing",
                easing,
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
