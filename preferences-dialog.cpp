#include "preferences-dialog.hpp"
#include "audio-input.hpp"
#include "audio-output.hpp"
#include "preferences.hpp"
#include "ui.hpp"
#include <SDL.h>
#include <imgui/imgui.h>
#include <log/log.hpp>

PreferencesDialog::PreferencesDialog(class Preferences &preferences,
                                     class AudioOutput &aAudioOutput,
                                     class AudioInput &aAudioInput,
                                     Cb cb)
  : Dialog("Preferences", std::move(cb)),
    preferences(preferences),
    audioOutput(aAudioOutput),
    audioInput(aAudioInput),
    audioLevel(aAudioInput)
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
      ImGui::Text("Twitch Settings");
      ImGui::TableNextColumn();
    }
    {
      ImGui::TableNextColumn();
      ImGui::Text("Twitch User:");
      ImGui::TableNextColumn();
      ImGui::PushItemWidth(ImGui::GetFontSize() * 40.f);
      char buf[1024];
      strcpy(buf, preferences.get().twitchUser.data());
      if (ImGui::InputText("##Twitch User", buf, sizeof(buf)))
        preferences.get().twitchUser = buf;
      ImGui::PopItemWidth();
    }

    {
      ImGui::TableNextColumn();
      ImGui::Text("Twitch Key:");
      ImGui::TableNextColumn();
      ImGui::PushItemWidth(ImGui::GetFontSize() * 40.f);
      char buf[1024];
      strcpy(buf, preferences.get().twitchKey.data());
      if (ImGui::InputText("##Twitch Key", buf, sizeof(buf), ImGuiInputTextFlags_Password))
        preferences.get().twitchKey = buf;
      ImGui::Text("e.g.: oauth:jfnrhghr8z3mfnbfeud8sk32dnf922");
      ImGui::PopItemWidth();
    }
    {
      ImGui::TableNextColumn();
      ImGui::Text("");
      ImGui::TableNextColumn();
    }
    {
      ImGui::TableNextColumn();
      ImGui::Text("Audio Settings");
      ImGui::TableNextColumn();
    }
    {
      ImGui::TableNextColumn();
      ImGui::Text("Output Device:");
      ImGui::TableNextColumn();
      {
        auto combo = Ui::Combo("##Output Device", preferences.get().outputAudio.c_str(), 0);
        if (combo)
        {
          if (ImGui::Selectable("Default##Output",
                                preferences.get().outputAudio == Preferences::DefaultAudio))
            updateOutputAudio(Preferences::DefaultAudio);
          const auto n = SDL_GetNumAudioDevices(0 /*output*/);
          for (auto i = 0; i < n; ++i)
          {
            auto dev = SDL_GetAudioDeviceName(i, 0 /*output*/);
            if (ImGui::Selectable((dev + std::string{"##Output"}).c_str(),
                                  preferences.get().outputAudio == dev))
              updateOutputAudio(dev);
          }
        }
      }
    }
    {
      ImGui::TableNextColumn();
      ImGui::Text("Input Device:");
      ImGui::TableNextColumn();
      {
        auto combo = Ui::Combo("##Input Device", preferences.get().inputAudio.c_str(), 0);
        if (combo)
        {
          if (ImGui::Selectable("Default##Input",
                                preferences.get().inputAudio == Preferences::DefaultAudio))
            updateInputAudio(Preferences::DefaultAudio);
          const auto n = SDL_GetNumAudioDevices(1 /*input*/);
          for (auto i = 0; i < n; ++i)
          {
            auto dev = SDL_GetAudioDeviceName(i, 1 /*input*/);
            if (ImGui::Selectable((dev + std::string{"##Input"}).c_str(),
                                  preferences.get().inputAudio == dev))
              updateInputAudio(dev);
          }
        }
      }
      ImGui::ProgressBar(audioLevel.getLevel(), ImVec2(0.0f, 0.0f));
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

auto PreferencesDialog::updateOutputAudio(std::string v) -> void
{
  preferences.get().outputAudio = std::move(v);
  audioOutput.get().updateDevice(preferences.get().outputAudio);
}

auto PreferencesDialog::updateInputAudio(std::string v) -> void
{
  preferences.get().inputAudio = std::move(v);
  audioInput.get().updateDevice(preferences.get().inputAudio);
}
