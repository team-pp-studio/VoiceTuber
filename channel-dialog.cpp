#include "channel-dialog.hpp"
#include <imgui/imgui.h>

ChannelDialog::ChannelDialog(std::string initChannel, Cb aCb)
  : Dialog("Enter Twitch Channel Name", [aCb = std::move(aCb), this](bool r) { aCb(r, channel); }),
    channel(std::move(initChannel))
{
}

auto ChannelDialog::internalDraw() -> DialogState
{
  char buf[1024];
  strcpy(buf, channel.data());
  if (ImGui::InputText("##channel", buf, sizeof(buf)))
    channel = buf;
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
