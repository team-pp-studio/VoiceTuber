#include "input-dialog.hpp"
#include <imgui/imgui.h>

InputDialog::InputDialog(std::string title, std::string init, Cb aCb)
  : Dialog(std::move(title), [aCb = std::move(aCb), this](bool r) { aCb(r, input); }),
    input(std::move(init))
{
}

auto InputDialog::internalDraw() -> DialogState
{
  char buf[1024];
  strcpy(buf, input.data());
  if (ImGui::InputText("##input", buf, sizeof(buf)))
    input = buf;
  const auto BtnSz = 90;
  ImGui::SameLine(700 - 2 * BtnSz - 10);
  if (ImGui::Button("OK", ImVec2(BtnSz, 0)))
    return DialogState::ok;
  ImGui::SetItemDefaultFocus();
  ImGui::SameLine();
  if (ImGui::Button("Cancel", ImVec2(BtnSz, 0)))
    return DialogState::cancel;
  return DialogState::active;
}
