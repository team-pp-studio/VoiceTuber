#include "bouncer.hpp"
#include <SDL_opengl.h>
#include <imgui/imgui.h>
#include <limits>
#include <log/log.hpp>

auto Bouncer::ingest(Wav v) -> void
{
  if (v.empty())
    return;
  offset = strength * v.back() / 0x8000;
}

auto Bouncer::render() -> void
{
  glTranslatef(.0f, offset, .0f);
}

auto Bouncer::renderUi() -> void
{
  ImGui::PushID("Bouncer");
  ImGui::PushItemWidth(ImGui::GetFontSize() * 16 + 8);
  ImGui::DragFloat("Bounce",
                   &strength,
                   1.f,
                   0.f,
                   std::numeric_limits<float>::max(),
                   "%.1f",
                   ImGuiSliderFlags_AlwaysClamp);
  ImGui::PopItemWidth();
  ImGui::PopID();
}
