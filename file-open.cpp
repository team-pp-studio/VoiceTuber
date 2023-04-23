#include "file-open.hpp"
#include <functional>
#include <imgui/imgui.h>
#include <log/log.hpp>

FileOpen::FileOpen(std::string dialogName, Cb aCb)
  : Dialog(std::move(dialogName), [this, aCb = std::move(aCb)](bool r) { aCb(r, getSelectedFile()); }),
    cwd(std::filesystem::current_path())
{
}

auto FileOpen::draw() -> bool
{
  if (!Dialog::draw())
    return false;

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
            ImGui::EndPopup();
            cb(true);
            return false;
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

  const auto BtnSz = 90;
  ImGui::SameLine(700 - 2 * BtnSz - 10);
  if (ImGui::Button("Open", ImVec2(BtnSz, 0)))
  {
    ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    cb(true);
    return false;
  }
  ImGui::SetItemDefaultFocus();
  ImGui::SameLine();
  if (ImGui::Button("Cancel", ImVec2(BtnSz, 0)))
  {
    ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    cb(false);
    return false;
  }
  ImGui::EndPopup();
  return true;
}

auto FileOpen::getSelectedFile() const -> std::filesystem::path
{
  return selectedFile;
}
