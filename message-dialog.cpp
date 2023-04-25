#include "message-dialog.hpp"
#include <imgui/imgui.h>

MessageDialog::MessageDialog(std::string title, std::string msg)
  : Dialog(std::move(title), [](bool) {}), msg(std::move(msg))
{
}

auto MessageDialog::internalDraw() -> DialogState
{
  ImGui::Text("%s", msg.c_str());
  const auto BtnSz = 90;
  ImGui::SameLine(700 - 2 * BtnSz - 10);
  if (ImGui::Button("OK", ImVec2(BtnSz, 0)))
    return DialogState::ok;
  ImGui::SetItemDefaultFocus();
  return DialogState::active;
}
