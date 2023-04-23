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
  return true;
}
