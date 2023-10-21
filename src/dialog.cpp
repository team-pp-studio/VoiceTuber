#include "dialog.hpp"
#include <imgui.h>

Dialog::Dialog(std::string title, Callback callback)
  : callback(std::move(callback)), title(std::move(title))
{
}

Dialog::~Dialog() {}

bool Dialog::draw()
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
    callback(true);
    return false;
  case DialogState::cancel:
    ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    callback(false);
    return false;
  }
  return true;
}
