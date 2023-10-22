#include "prj-dialog.hpp"
#include "imgui-helpers.hpp"
#include "ui.hpp"
#include "version.hpp"
#include <algorithm>
#include <fmt/std.h>
#include <functional>
#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <windows.h>
#endif

PrjDialog::PrjDialog(Lib &lib, Callback callback)
  : Dialog("New/Open Project", std::move(callback)),
    cwd(std::filesystem::current_path()),
    upDir(lib.queryTex("engine:up-dir.png", true)),
    bckDir(lib.queryTex("engine:arrow-left.png", true)),
    fwdDir(lib.queryTex("engine:arrow-right.png", true)),
    splash(lib.queryTex("engine:splashscreen.png", true)),
    donate(lib.queryTex("engine:donate.png", true)),
    github(lib.queryTex("engine:github.png", true))
{
}

auto PrjDialog::internalDraw() -> DialogState
{
  ImGui::Image((void *)(intptr_t)splash->texture(),
               ImVec2{static_cast<float>(splash->w()), static_cast<float>(splash->h())});
  auto availableSpace =
    ImVec2{static_cast<float>(splash->w() - 240), static_cast<float>(splash->h() - 180 - 265 + 58)};

  ImVec2 listBoxSize(availableSpace.x, availableSpace.y - 30 - 64);

  ImGui::SetCursorPos(ImVec2{30, 287});
  auto hasSelected = false;
  const auto sz = ImGui::GetFontSize();
  {
    auto disable = Ui::Disabled{undoStack.empty()};
    if (ImGui::ImageButton((void *)(intptr_t)bckDir->texture(), ImVec2(sz, sz)))
    {
      auto d = undoStack.back();
      undoStack.pop_back();
      redoStack.push_back(cwd);
      cwd = d;
      dirs.clear();
      selectedDir = "";
    }
  }
  ImGui::SameLine();
  {
    auto disable = Ui::Disabled{redoStack.empty()};
    if (ImGui::ImageButton((void *)(intptr_t)fwdDir->texture(), ImVec2(sz, sz)))
    {
      auto d = redoStack.back();
      redoStack.pop_back();
      undoStack.push_back(cwd);
      cwd = d;
      dirs.clear();
      selectedDir = "";
    }
  }
  ImGui::SameLine();
  if (ImGui::ImageButton((void *)(intptr_t)upDir->texture(), ImVec2(sz, sz)))
  {
    dirs.clear();
    selectedDir = "";
    undoStack.push_back(cwd);
    redoStack.clear();
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
  {
    char buf[4096];
    strcpy(buf, cwd.string().data());
    ImGui::InputText("##current directory", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
  }

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
        SPDLOG_ERROR("{:t}", e);
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

  ImGui::SetCursorPosX(30);
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
          undoStack.push_back(cwd);
          redoStack.clear();
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
  ImGui::SetCursorPosX(30);
  if (ImGui::InputText("##dirname", buf, sizeof(buf)))
    selectedDir = buf;
  ImGui::PopItemWidth();

  ImGui::SameLine();
  if (hasSelected)
  {
    if (ImGui::Button("Open", ImVec2{BtnSz, 0}))
    {
      undoStack.push_back(cwd);
      redoStack.clear();
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

  ImGui::SetCursorPos(ImVec2{1110 - 160 + 20, 412 - 142 + 30});
  if (ImGui::ImageButton((void *)(intptr_t)donate->texture(),
                         ImVec2{static_cast<float>(donate->w()), static_cast<float>(donate->h())}))
    SDL_OpenURL("https://github.com/sponsors/WichitPritchett");
  ImGui::SetCursorPosX(1110 - 160 + 20);
  if (ImGui::ImageButton((void *)(intptr_t)github->texture(),
                         ImVec2{static_cast<float>(github->w()), static_cast<float>(github->h())}))
    SDL_OpenURL("https://github.com/team-pp-studio/VoiceTuber");

  ImGui::SetCursorPos(ImVec2{226, 140 + 15});
  ImGui::TextF("v{}", appVersion());

  return DialogState::active;
}
