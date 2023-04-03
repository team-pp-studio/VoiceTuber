#include "prj-dialog.hpp"
#include <functional>
#include <imgui/imgui.h>

auto PrjDialog::draw() -> bool
{
  ImGui::Begin("New/Open Project");
  if (dirs.empty())
  {
    const auto cwd = std::filesystem::current_path();
    for (auto &entry : std::filesystem::directory_iterator(cwd))
      if (entry.is_directory())
        dirs.push_back(entry.path());
  }

  auto ret = false;
  auto hasSelected = false;
  if (ImGui::BeginListBox("##dirs", ImVec2(700, 400)))
  {
    std::function<void()> postponedAction = nullptr;
    if (ImGui::Selectable("..", ".." == selectedDir, ImGuiSelectableFlags_AllowDoubleClick))
      if (ImGui::IsMouseDoubleClicked(0))
        postponedAction = [this]() {
          dirs.clear();
          selectedDir = "";
          const auto cwd = std::filesystem::current_path();
          std::filesystem::current_path(cwd.parent_path());
        };

    for (auto &dir : dirs)
    {
      if (selectedDir == dir.filename())
        hasSelected = true;
      if (ImGui::Selectable(("> " + dir.filename().string()).c_str(),
                            selectedDir == dir.filename(),
                            ImGuiSelectableFlags_AllowDoubleClick))
      {
        if (ImGui::IsMouseDoubleClicked(0))
          postponedAction = [this, dir]() {
            std::filesystem::current_path(dir);
            dirs.clear();
            selectedDir = "";
          };
        else
          selectedDir = dir.filename();
      }
    }
    if (postponedAction)
      postponedAction();
    ImGui::EndListBox();
  }

  char buf[1024];
  strcpy(buf, selectedDir.data());
  ImGui::InputText("##dirname", buf, sizeof(buf));
  selectedDir = buf;
  ImGui::SameLine();
  if (hasSelected)
  {
    if (ImGui::Button("Open"))
    {
      std::filesystem::current_path(selectedDir);
      dirs.clear();
      selectedDir = "";
      ret = true;
    }
  }
  else
  {
    ImGui::BeginDisabled(selectedDir.empty());
    if (ImGui::Button("New"))
    {
      if (std::filesystem::create_directories(selectedDir))
      {
        std::filesystem::current_path(selectedDir);
        dirs.clear();
        selectedDir = "";
        ret = true;
      }
    }
    ImGui::EndDisabled();
  }
  ImGui::End();
  return ret;
}
