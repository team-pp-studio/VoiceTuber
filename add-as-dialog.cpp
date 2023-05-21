#include "add-as-dialog.hpp"
#include <imgui/imgui.h>

AddAsDialog::AddAsDialog(std::string aPath, Cb aCb)
  : Dialog("Add As...", [aCb = std::move(aCb), this](bool r) { aCb(r, nodeType); }),
    path(std::move(aPath))
{
}

auto AddAsDialog::internalDraw() -> DialogState
{
  ImGui::Text("Add \"%s\" as...", path.c_str());
  if (ImGui::RadioButton("Sprite", nodeType == NodeType::sprite))
    nodeType = NodeType::sprite;
  ImGui::SameLine();
  if (ImGui::RadioButton("Mouth", nodeType == NodeType::mouth))
    nodeType = NodeType::mouth;
  ImGui::SameLine();
  if (ImGui::RadioButton("Eye", nodeType == NodeType::eye))
    nodeType = NodeType::eye;

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
