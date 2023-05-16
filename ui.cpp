#include "ui.hpp"
#include "texture.hpp"
#include <imgui/imgui.h>

namespace Ui
{
  auto textRj(const std::string &v, float offset) -> void
  {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() -
                         ImGui::CalcTextSize(v.c_str()).x - ImGui::GetScrollX() -
                         2 * ImGui::GetStyle().ItemSpacing.x - offset);
    ImGui::Text("%s", v.c_str());
  }

  auto BtnImg(const std::string &id, const class Texture &t, float w, float h) -> bool
  {
    return ImGui::ImageButton(id.c_str(), (void *)(intptr_t)t.texture(), ImVec2(w, h));
  }
} // namespace Ui
