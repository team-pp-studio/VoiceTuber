#include "preferences-dialog.hpp"
#include "preferences.hpp"
#include "ui.hpp"
#include <imgui/imgui.h>

PreferencesDialog::PreferencesDialog(class Preferences &preferences, Cb cb)
  : Dialog("Preferences", std::move(cb)), preferences(preferences)
{
}

auto PreferencesDialog::internalDraw() -> DialogState
{
  const auto BtnSz = 90;
  if (auto preferencesTable = Ui::Table{"##preferences",
                                        2,
                                        ImGuiTableFlags_SizingFixedFit,
                                        ImVec2{ImGui::GetFontSize() * (40.f + 8.f), 100.f}})
  {
    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 8.f);
    ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFontSize() * 40.f);
    {
      ImGui::TableNextColumn();
      char buf[1024];
      strcpy(buf, preferences.get().twitchUser.data());
      ImGui::Text("Twitch User:");
      ImGui::TableNextColumn();
      ImGui::PushItemWidth(ImGui::GetFontSize() * 40.f);
      if (ImGui::InputText("##Twitch User", buf, sizeof(buf)))
        preferences.get().twitchUser = buf;
      ImGui::PopItemWidth();
    }

    {
      ImGui::TableNextColumn();
      char buf[1024];
      strcpy(buf, preferences.get().twitchKey.data());
      ImGui::Text("Twitch Key:");
      ImGui::TableNextColumn();
      ImGui::PushItemWidth(ImGui::GetFontSize() * 40.f);
      if (ImGui::InputText("##Twitch Key", buf, sizeof(buf), ImGuiInputTextFlags_Password))
        preferences.get().twitchKey = buf;
      ImGui::PopItemWidth();
    }
  }

  ImGui::SetCursorPosX(ImGui::GetWindowWidth() - BtnSz - ImGui::GetStyle().WindowPadding.x);
  if (ImGui::Button("OK", ImVec2(BtnSz, 0)))
  {
    preferences.get().save();
    return DialogState::ok;
  }
  ImGui::SetItemDefaultFocus();
  return DialogState::active;
}
