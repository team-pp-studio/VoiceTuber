#include "preferences-dialog.hpp"
#include "audio-in.hpp"
#include "audio-out.hpp"
#include "preferences.hpp"
#include "ui.hpp"
#include <SDL.h>
#include <imgui.h>
#include <log/log.hpp>

PreferencesDialog::PreferencesDialog(class Preferences &preferences,
                                     class AudioOut &aAudioOut,
                                     class AudioIn &aAudioIn,
                                     Cb cb)
  : Dialog("Preferences", std::move(cb)),
    preferences(preferences),
    audioOut(aAudioOut),
    audioIn(aAudioIn),
    audioLevel(aAudioIn)
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
      Ui::textRj("Twitch User:");
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
      Ui::textRj("Twitch Key:");
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
      Ui::textRj("Azure Key:");
      ImGui::TableNextColumn();
      ImGui::PushItemWidth(ImGui::GetFontSize() * 40.f);
      char buf[1024];
      strcpy(buf, preferences.get().azureKey.data());
      if (ImGui::InputText("##Azure TTS Key", buf, sizeof(buf), ImGuiInputTextFlags_Password))
        preferences.get().azureKey = buf;
      ImGui::Text("e.g.: 1e3b7527b4e3ec61dee69a83979ef9d6");
      ImGui::PopItemWidth();
    }
    {
      ImGui::TableNextColumn();
      Ui::textRj("Open AI Token:");
      ImGui::TableNextColumn();
      ImGui::PushItemWidth(ImGui::GetFontSize() * 40.f);
      char buf[1024];
      strcpy(buf, preferences.get().openAiToken.data());
      if (ImGui::InputText("##Open AI Token", buf, sizeof(buf), ImGuiInputTextFlags_Password))
        preferences.get().openAiToken = buf;
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
      Ui::textRj("Output Device:");
      ImGui::TableNextColumn();
      {
        auto combo = Ui::Combo("##Output Device", preferences.get().audioOut.c_str(), 0);
        if (combo)
        {
          if (ImGui::Selectable("Default##Output",
                                preferences.get().audioOut == Preferences::DefaultAudio))
            updateAudioOut(Preferences::DefaultAudio);
          const auto n = SDL_GetNumAudioDevices(0 /*output*/);
          for (auto i = 0; i < n; ++i)
          {
            auto dev = SDL_GetAudioDeviceName(i, 0 /*output*/);
            if (ImGui::Selectable((dev + std::string{"##Output"}).c_str(),
                                  preferences.get().audioOut == dev))
              updateAudioOut(dev);
          }
        }
      }
    }
    {
      ImGui::TableNextColumn();
      Ui::textRj("Input Device:");
      ImGui::TableNextColumn();
      {
        auto combo = Ui::Combo("##Input Device", preferences.get().audioIn.c_str(), 0);
        if (combo)
        {
          if (ImGui::Selectable("Default##Input",
                                preferences.get().audioIn == Preferences::DefaultAudio))
            updateAudioIn(Preferences::DefaultAudio);
          const auto n = SDL_GetNumAudioDevices(1 /*input*/);
          for (auto i = 0; i < n; ++i)
          {
            auto dev = SDL_GetAudioDeviceName(i, 1 /*input*/);
            if (ImGui::Selectable((dev + std::string{"##Input"}).c_str(),
                                  preferences.get().audioIn == dev))
              updateAudioIn(dev);
          }
        }
      }
      ImGui::ProgressBar(audioLevel.getLevel(), ImVec2(0.0f, 0.0f));
    }

    {
      ImGui::TableNextColumn();
      ImGui::Text("Graphics Settings");
      ImGui::TableNextColumn();
    }
    {
      ImGui::TableNextColumn();
      Ui::textRj("VSync:");
      ImGui::TableNextColumn();
      if (ImGui::Checkbox("##VSync", &preferences.get().vsync))
        SDL_GL_SetSwapInterval(preferences.get().vsync ? 1 : 0);
    }
    {
      ImGui::TableNextColumn();
      Ui::textRj("FPS:");
      ImGui::TableNextColumn();
      ImGui::DragInt("0 = unbounded##fps", &preferences.get().fps, 1, 0, 240);
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

auto PreferencesDialog::updateAudioOut(std::string v) -> void
{
  preferences.get().audioOut = std::move(v);
  audioOut.get().updateDevice(preferences.get().audioOut);
}

auto PreferencesDialog::updateAudioIn(std::string v) -> void
{
  preferences.get().audioIn = std::move(v);
  audioIn.get().updateDevice(preferences.get().audioIn);
}
