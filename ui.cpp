#include "ui.hpp"
#include "texture.hpp"
#include "undo.hpp"
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

  auto btnImg(const std::string &id, const class Texture &t, float w, float h) -> bool
  {
    return ImGui::ImageButton(id.c_str(), (void *)(intptr_t)t.texture(), ImVec2(w, h));
  }

  auto dragFloat(class Undo &undo,
                 const char *label,
                 float &v,
                 float v_speed,
                 float v_min,
                 float v_max,
                 const char *format,
                 ImGuiSliderFlags flags) -> bool
  {
    const auto oldV = v;
    if (ImGui::DragFloat(label, &v, v_speed, v_min, v_max, format, flags))
    {
      undo.record([newV = v, &v]() { v = newV; }, [oldV, &v]() { v = oldV; }, label);
      return true;
    }
    return false;
  }

  auto inputInt(Undo &undo,
                const char *label,
                int &v,
                int step,
                int step_fast,
                ImGuiInputTextFlags flags) -> bool
  {
    const auto oldV = v;
    if (ImGui::InputInt(label, &v, step, step_fast, flags))
    {
      undo.record([newV = v, &v]() { v = newV; }, [oldV, &v]() { v = oldV; }, label);
      return true;
    }
    return false;
  }

  auto sliderFloat(Undo &undo,
                   const char *label,
                   float &v,
                   float v_min,
                   float v_max,
                   const char *format,
                   ImGuiSliderFlags flags) -> bool
  {
    const auto oldV = v;
    if (ImGui::SliderFloat(label, &v, v_min, v_max, format, flags))
    {
      undo.record([newV = v, &v]() { v = newV; }, [oldV, &v]() { v = oldV; }, label);
      return true;
    }
    return false;
  }

  auto checkbox(Undo &undo, const char *label, bool &v) -> bool
  {
    const auto oldV = v;
    if (ImGui::Checkbox(label, &v))
    {
      undo.record([newV = v, &v]() { v = newV; }, [oldV, &v]() { v = oldV; });
      return true;
    }
    return false;
  }

} // namespace Ui
