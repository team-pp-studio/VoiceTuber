#include "prj-dialog.hpp"
#include <functional>
#include <imgui/imgui.h>
#include <log/log.hpp>

PrjDialog::PrjDialog(Cb aCb) : Dialog("New/Open Project", std::move(aCb)) {}

auto PrjDialog::internalDraw() -> DialogState
{
  auto availableSpace = ImGui::GetContentRegionAvail();
  availableSpace.x = std::max(availableSpace.x, 500.f);
  availableSpace.y = std::max(availableSpace.y, 250.f);
  // Adjust the width of the ListBox
  ImVec2 listBoxSize(availableSpace.x, availableSpace.y - 30); // Adjust the width and height as needed
  if (dirs.empty())
  {
    const auto cwd = std::filesystem::current_path();
    for (auto &entry : std::filesystem::directory_iterator(cwd))
      if (entry.is_directory())
        dirs.push_back(entry.path());
    std::sort(std::begin(dirs), std::end(dirs));
  }

  auto hasSelected = false;
  if (ImGui::BeginListBox("##dirs", listBoxSize))
  {
    const auto oldSelectedDir = selectedDir;
    if (ImGui::Selectable("..", ".." == oldSelectedDir, ImGuiSelectableFlags_AllowDoubleClick))
      if (ImGui::IsMouseDoubleClicked(0))
      {
        dirs.clear();
        selectedDir = "";
        const auto cwd = std::filesystem::current_path();
        std::filesystem::current_path(cwd.parent_path());
      }

    for (auto &dir : dirs)
    {
      if (oldSelectedDir == dir.filename())
        hasSelected = true;
      if (ImGui::Selectable(("> " + dir.filename().string()).c_str(),
                            oldSelectedDir == dir.filename(),
                            ImGuiSelectableFlags_AllowDoubleClick))
      {
        if (ImGui::IsMouseDoubleClicked(0))
        {
          std::filesystem::current_path(dir);
          dirs.clear();
          selectedDir = "";
          const auto projectFilePath = std::filesystem::path(selectedDir) / "prj.tpp";
          if (std::filesystem::exists(projectFilePath))
          {
            ImGui::EndListBox();
            return DialogState::ok;
          }
          break;
        }
        else
          selectedDir = dir.filename().string();
      }
    }
    ImGui::EndListBox();
  }

  const auto BtnSz = 90.f;
  ImGui::PushItemWidth(availableSpace.x - BtnSz - 10);
  char buf[1024];
  strcpy(buf, selectedDir.data());
  if (ImGui::InputText("##dirname", buf, sizeof(buf)))
    selectedDir = buf;
  ImGui::PopItemWidth();

  ImGui::SameLine();
  if (hasSelected)
  {
    if (ImGui::Button("Open", ImVec2{BtnSz, 0}))
    {
      std::filesystem::current_path(selectedDir);
      dirs.clear();
      selectedDir = "";
      const auto projectFilePath = std::filesystem::path(selectedDir) / "prj.tpp";
      if (std::filesystem::exists(projectFilePath))
        return DialogState::ok;
    }
  }
  else
  {
    ImGui::BeginDisabled(selectedDir.empty());
    if (ImGui::Button("New", ImVec2{BtnSz, 0}))
    {
      if (std::filesystem::create_directories(selectedDir))
      {
        std::filesystem::current_path(selectedDir);
        dirs.clear();
        selectedDir = "";
        ImGui::EndDisabled();
        return DialogState::ok;
      }
    }
    ImGui::EndDisabled();
  }
  return DialogState::active;
}
