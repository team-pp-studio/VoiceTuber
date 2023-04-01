#include "anim-sprite.hpp"
#include <imgui/imgui.h>
#include <log/log.hpp>

AnimSprite::AnimSprite(const std::string &fileName)
  : Sprite(fileName), startTime(std::chrono::high_resolution_clock::now())
{
}

auto AnimSprite::render(Node *hovered, Node *selected) -> void
{
  frame = static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(
                             std::chrono::high_resolution_clock::now() - startTime)
                             .count() *
                           fps / 1'000'000) %
          numFrames;
  Sprite::render(hovered, selected);
}

auto AnimSprite::renderUi() -> void
{
  Sprite::renderUi();
  ImGui::PushID("AnimSprite");
  ImGui::PushItemWidth(ImGui::GetFontSize() * 16 + 8);
  ImGui::DragFloat(
    "FPS", &fps, 1, 1, std::numeric_limits<float>::max(), "%.1f", ImGuiSliderFlags_AlwaysClamp);
  ImGui::PopItemWidth();
  ImGui::PopID();
}
