// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal
// graphics context creation, etc.) If you are new to Dear ImGui, read documentation from the docs/
// folder + read the top of imgui.cpp. Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "app.hpp"
#include <chrono>
#include <filesystem>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <sdlpp/sdlpp.hpp>
#include <stdio.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

// Main code
int main(int argc, char **argv)
{
  // Setup SDL
  auto init = sdl::Init{SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO};
  SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

  std::filesystem::current_path(sdl::get_base_path());

  // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

  // Create window with graphics context
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_WindowFlags window_flags =
    (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

  auto window =
    sdl::Window{"VoiceTuber", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, window_flags};

  auto appIco = sdl::Surface("assets/app-icon.bmp");
  window.setIcon(appIco.get());

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
                                                        // io.ConfigViewportsNoAutoMerge = true;
                                                        // io.ConfigViewportsNoTaskBarIcon = true;

  io.ConfigWindowsMoveFromTitleBarOnly = true;

  // Setup Dear ImGui style
  // ImGui::StyleColorsDark();
  ImGui::StyleColorsLight();
  // ImGui::StyleColorsClassic();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical
  // to regular ones.
  ImGuiStyle &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    style.WindowRounding = 0.f;
    style.FrameRounding = 5.f;
    style.Colors[ImGuiCol_Text] = ImVec4{0xff / 255.f, 0xff / 255.f, 0xff / 255.f, 1.f};
    style.Colors[ImGuiCol_TextDisabled] = ImVec4{0xc0 / 255.f, 0xc0 / 255.f, 0xc0 / 255.f, 1.f};
    style.Colors[ImGuiCol_WindowBg] = ImVec4{0x7f / 255.f, 0x8d / 255.f, 0xba / 255.f, .8f};
    style.Colors[ImGuiCol_ChildBg] = ImVec4{0x7f / 255.f, 0x8d / 255.f, 0xba / 255.f, 1.f};
    style.Colors[ImGuiCol_PopupBg] = ImVec4{0x7f / 255.f, 0x8d / 255.f, 0xba / 255.f, 1.f};
    style.Colors[ImGuiCol_Border] = ImVec4{0x6f / 255.f, 0x7d / 255.f, 0xaa / 255.f, 1.f};
    style.Colors[ImGuiCol_BorderShadow] = ImVec4{0x56 / 255.f, 0x58 / 255.f, 0x8d / 255.f, 1.f};
    style.Colors[ImGuiCol_FrameBg] = ImVec4{0x6f / 255.f, 0x7d / 255.f, 0xaa / 255.f, 1.f};
    // ImGuiCol_FrameBgHovered,
    // ImGuiCol_FrameBgActive,
    style.Colors[ImGuiCol_TitleBg] = ImVec4{0x7f / 255.f, 0x8d / 255.f, 0xba / 255.f, 1.f};
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0xd3 / 255.f, 0xac / 255.f, 0xcb / 255.f, .1f};
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4{0x8f / 255.f, 0x9d / 255.f, 0xca / 255.f, 1.f};
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4{0x7f / 255.f, 0x8d / 255.f, 0xba / 255.f, 1.f};
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4{0x7f / 255.f, 0x8d / 255.f, 0xba / 255.f, 1.f};
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4{0xbb / 255.f, 0xc4 / 255.f, 0xdc / 255.f, 1.f};
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{0xcb / 255.f, 0xd4 / 255.f, 0xec / 255.f, 1.f};
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{0xdb / 255.f, 0xe4 / 255.f, 0xfc / 255.f, 1.f};
    style.Colors[ImGuiCol_CheckMark] = ImVec4{0xbb / 255.f, 0xc4 / 255.f, 0xdc / 255.f, 1.f};
    style.Colors[ImGuiCol_SliderGrab] = ImVec4{0xbb / 255.f, 0xc4 / 255.f, 0xdc / 255.f, 1.f};
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4{0xcb / 255.f, 0xd4 / 255.f, 0xec / 255.f, 1.f};
    style.Colors[ImGuiCol_Button] = ImVec4{0xd3 / 255.f, 0xac / 255.f, 0xcb / 255.f, 1.f};
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4{0xe3 / 255.f, 0xbc / 255.f, 0xdb / 255.f, 1.f};
    style.Colors[ImGuiCol_ButtonActive] = ImVec4{0xf3 / 255.f, 0xcc / 255.f, 0xeb / 255.f, 1.f};
    style.Colors[ImGuiCol_Header] = ImVec4{0xd3 / 255.f, 0xac / 255.f, 0xcb / 255.f, 1.f};
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4{0xe3 / 255.f, 0xbc / 255.f, 0xdb / 255.f, 1.f};
    style.Colors[ImGuiCol_HeaderActive] = ImVec4{0xf3 / 255.f, 0xcc / 255.f, 0xeb / 255.f, 1.f};
    // ImGuiCol_Separator,
    // ImGuiCol_SeparatorHovered,
    // ImGuiCol_SeparatorActive,
    // ImGuiCol_ResizeGrip,
    // ImGuiCol_ResizeGripHovered,
    // ImGuiCol_ResizeGripActive,
    style.Colors[ImGuiCol_Tab] = ImVec4{0xb1 / 255.f, 0x9e / 255.f, 0xc2 / 255.f, 1.f};
    style.Colors[ImGuiCol_TabHovered] = ImVec4{0xe3 / 255.f, 0xbc / 255.f, 0xdb / 255.f, 1.f};
    style.Colors[ImGuiCol_TabActive] = ImVec4{0xd3 / 255.f, 0xac / 255.f, 0xcb / 255.f, 1.f};
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4{0xb1 / 255.f, 0x9e / 255.f, 0xc2 / 255.f, 1.f};
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0xd3 / 255.f, 0xac / 255.f, 0xcb / 255.f, 1.f};
    // ImGuiCol_DockingPreview,        // Preview overlay color when about to docking something
    // ImGuiCol_DockingEmptyBg,
    // ImGuiCol_PlotLines,
    // ImGuiCol_PlotLinesHovered,
    // ImGuiCol_PlotHistogram,
    // ImGuiCol_PlotHistogramHovered,
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4{0x6f / 255.f, 0x7d / 255.f, 0xaa / 255.f, 1.f};
    // ImGuiCol_TableBorderStrong,     // Table outer and header borders (prefer using Alpha=1.0 here)
    // ImGuiCol_TableBorderLight,      // Table inner borders (prefer using Alpha=1.0 here)
    // ImGuiCol_TableRowBg,            // Table row background (even rows)
    // ImGuiCol_TableRowBgAlt,         // Table row background (odd rows)
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4{0xd3 / 255.f, 0xac / 255.f, 0xcb / 255.f, 1.f};
    // ImGuiCol_DragDropTarget,        // Rectangle highlighting a drop target
    // ImGuiCol_NavHighlight,          // Gamepad/keyboard: current highlighted item
    // ImGuiCol_NavWindowingHighlight, // Highlight window when using CTRL+TAB
    // ImGuiCol_NavWindowingDimBg,
    // style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4{0x6f / 255.f, 0x7d / 255.f, 0xaa / 255.f, 1.f};
  }

  auto app = App{window, argc, argv};

#ifdef __EMSCRIPTEN__
  // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen()
  // of the imgui.ini file. You may manually call LoadIniSettingsFromMemory() to load settings from
  // your own storage.
  io.IniFilename = NULL;
  EMSCRIPTEN_MAINLOOP_BEGIN
#else
  while (!app.done)
#endif
  {
    app.tick();
  }
#ifdef __EMSCRIPTEN__
  EMSCRIPTEN_MAINLOOP_END;
#endif

  return 0;
}
