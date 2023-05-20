#include "dialog.hpp"
#include <imgui/imgui.h>

Dialog::Dialog(std::string aTitle, Cb cb) : cb(std::move(cb)), title(std::move(aTitle)) {}

auto Dialog::draw() -> bool
{
  if (first)
  {
    ImGui::OpenPopup(title.c_str());
    first = false;
  }
  if (!ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    return false;

  switch (internalDraw())
  {
  case DialogState::active: ImGui::EndPopup(); return true;
  case DialogState::ok:
    ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    cb(true);
    return false;
  case DialogState::cancel:
    ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    cb(false);
    return false;
  }
  return true;
}
