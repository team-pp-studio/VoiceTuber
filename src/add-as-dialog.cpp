#include "add-as-dialog.hpp"
#include "imgui-helpers.hpp"

#include <fmt/std.h>

AddAsDialog::AddAsDialog(std::filesystem::path path, Callback callback)
  : Dialog("Add As...",
           [callback = std::move(callback), this](bool r) mutable { callback(r, nodeType); }),
    path(std::move(path))
{
}

auto AddAsDialog::internalDraw() -> DialogState
{
  ImGui::TextF("Add {:?} as...", path);
  char const *options[] = {
    "Sprite",
    "Mouth",
    "Eye",
    "AiMouth",
  };
  if (int index = 0; ImGui::Combo("Add as", &index, &options[0], std::size(options)))
  {
    nodeType = static_cast<NodeType>(index);
  }

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
