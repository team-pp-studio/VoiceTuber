#include "prj-dialog.hpp"
#include "ui.hpp"
#include <algorithm>
#include <functional>
#include <log/log.hpp>

#ifdef _WIN32
#include <Windows.h>
#endif

PrjDialog::PrjDialog(Lib &lib, Cb aCb)
  : Dialog("New/Open Project", std::move(aCb)),
    cwd(std::filesystem::current_path()),
    upDir(lib.queryTex("engine:up-dir.png", true))
{
}

auto PrjDialog::internalDraw() -> DialogState
{
  auto availableSpace = ImGui::GetContentRegionAvail();
  availableSpace.x = std::max(availableSpace.x, 700.f);
  availableSpace.y = std::max(availableSpace.y, 250.f + 64 + 30);
  // Adjust the width of the ListBox
  ImVec2 listBoxSize(availableSpace.x,
                     availableSpace.y - 30 - 64); // Adjust the width and height as needed

  auto hasSelected = false;
  const auto sz = ImGui::GetFontSize();
  if (ImGui::ImageButton((void *)(intptr_t)upDir->texture(), ImVec2(sz, sz)))
  {
    dirs.clear();
    selectedDir = "";
#ifdef _WIN32
    if (cwd != cwd.parent_path())
      cwd = cwd.parent_path();
    else
      cwd = "";
#else
    cwd = cwd.parent_path();
#endif
  }
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Go Up");
  ImGui::SameLine();
  ImGui::Text("%s", cwd.string().c_str());

  if (dirs.empty())
  {
    if (!cwd.string().empty())
    {
      try
      {
        for (auto &entry : std::filesystem::directory_iterator(cwd))
          if (entry.is_directory())
            dirs.push_back(entry.path());
      }
      catch (std::runtime_error &e)
      {
        LOG(e.what());
      }
      std::sort(std::begin(dirs), std::end(dirs));
    }
    else
    {
#ifdef _WIN32
      char d = 'A';
      for (auto drives = GetLogicalDrives(); drives != 0; drives >>= 1, ++d)
      {
        if (drives & 0x1)
        {
          auto drive = d + std::string{":\\"};
          dirs.push_back(drive);
        }
      }
#endif
    }
  }

  if (auto dirsListBox = Ui::ListBox{"##dirs", listBoxSize})
  {
    const auto oldSelectedDir = selectedDir;

    for (auto &dir : dirs)
    {
      const auto dirStr = [&dir]() {
        if (dir.filename().string().empty())
          return dir.string();
        else
          return dir.filename().string();
      }();
      if (oldSelectedDir == dirStr)
        hasSelected = true;
      if (ImGui::Selectable(
            ("> " + dirStr).c_str(), oldSelectedDir == dirStr, ImGuiSelectableFlags_AllowDoubleClick))
      {
        if (ImGui::IsMouseDoubleClicked(0))
        {
          cwd = dir;
          dirs.clear();
          selectedDir = "";
          const auto projectFilePath = cwd / "prj.tpp";
          if (std::filesystem::exists(projectFilePath))
          {
            std::filesystem::current_path(cwd);
            return DialogState::ok;
          }
          break;
        }
        else
          selectedDir = dirStr;
      }
    }
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
      cwd = cwd / selectedDir;
      dirs.clear();
      selectedDir = "";

      const auto projectFilePath = cwd / "prj.tpp";
      if (std::filesystem::exists(projectFilePath))
      {
        std::filesystem::current_path(cwd);
        return DialogState::ok;
      }
    }
  }
  else
  {
    auto newBtnDisabled = Ui::Disabled(selectedDir.empty());
    if (ImGui::Button("New", ImVec2{BtnSz, 0}))
    {
      cwd = cwd / selectedDir;
      if (std::filesystem::create_directories(cwd))
      {
        std::filesystem::current_path(cwd);
        dirs.clear();
        selectedDir = "";
        return DialogState::ok;
      }
    }
  }
  return DialogState::active;
}
