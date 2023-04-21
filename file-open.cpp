#include "file-open.hpp"
#include <functional>
#include <imgui/imgui.h>
#include <log/log.hpp>

FileOpen::FileOpen(const char *dialogName, Cb cb)
  : Dialog([this, cb = std::move(cb)] { cb(getSelectedFile()); }),
    dialogName(dialogName),
    cwd(std::filesystem::current_path())
{
}

auto FileOpen::draw() -> bool
{
  if (!ImGui::BeginPopupModal("modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    return false;

  auto ret = false;

  if (files.empty())
  {
    // list files and directories in the current directory
    for (auto &entry : std::filesystem::directory_iterator(cwd))
      files.push_back(entry.path());
    std::sort(files.begin(), files.end());
  }

  // Populate the files in the list box
  if (ImGui::BeginListBox("##files", ImVec2(700, 400)))
  {
    std::function<void()> postponedAction = nullptr;
    if (ImGui::Selectable("..", ".." == selectedFile, ImGuiSelectableFlags_AllowDoubleClick))
      if (ImGui::IsMouseDoubleClicked(0))
        postponedAction = [this]() {
          files.clear();
          selectedFile = "";
          cwd = cwd.parent_path();
        };

    for (auto &file : files)
    {
      const auto isHidden = file.filename().string().front() == '.';
      if (isHidden)
        continue;
      const auto isDirectory = std::filesystem::is_directory(file);

      if (ImGui::Selectable(((isDirectory ? "> " : "  ") + file.filename().string()).c_str(),
                            selectedFile == file,
                            ImGuiSelectableFlags_AllowDoubleClick))
      {
        if (ImGui::IsMouseDoubleClicked(0))
        {
          if (isDirectory)
          {
            // change directory
            postponedAction = [this, file]() {
              cwd = file;
              files.clear();
              selectedFile = "";
            };
          }
          else
          {
            selectedFile = file;
            ImGui::CloseCurrentPopup();
            ret = true;
          }
        }
        else
        {
          selectedFile = file;
        }
      }
    }
    if (postponedAction)
      postponedAction();
    ImGui::EndListBox();
  }

  // Show the selected file
  if (!selectedFile.empty())
    ImGui::Text("%s", selectedFile.filename().c_str());
  else
    ImGui::Text("No file selected");

  ImGui::SameLine(700 - 2 * 120 - 10);
  if (ImGui::Button("Open", ImVec2(120, 0)))
  {
    ImGui::CloseCurrentPopup();
    ret = true;
  }
  ImGui::SetItemDefaultFocus();
  ImGui::SameLine(700 - 120);
  if (ImGui::Button("Cancel", ImVec2(120, 0)))
    ImGui::CloseCurrentPopup();
  ImGui::EndPopup();
  if (ret)
    cb();
  return ret;
}

auto FileOpen::getSelectedFile() const -> std::filesystem::path
{
  return selectedFile;
}
