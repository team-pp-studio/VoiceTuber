#include "dialog.hpp"
#include <imgui/imgui.h>

Dialog::Dialog(std::string aTitle, Cb cb) : cb(std::move(cb)), title(std::move(aTitle))
{
  ImGui::OpenPopup(title.c_str());
}

auto Dialog::draw() -> bool
{
  if (!ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    return false;
  return true;
}
