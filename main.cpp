// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal
// graphics context creation, etc.) If you are new to Dear ImGui, read documentation from the docs/
// folder + read the top of imgui.cpp. Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "app.hpp"
#include "imgui-impl-opengl3.h"
#include "imgui-impl-sdl.h"
#include <chrono>
#include <filesystem>
#include <imgui/imgui.h>
#include <log/log.hpp>
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
int main(int, char **)
{
  // Setup SDL
  auto init = sdl::Init{SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO};

  std::filesystem::current_path(SDL_GetBasePath());

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  const char *glsl_version = "#version 100";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
  // GL 3.2 Core + GLSL 150
  const char *glsl_version = "#version 150";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                      SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

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
  SDL_GLContext gl_context = SDL_GL_CreateContext(window.get());
  SDL_GL_MakeCurrent(window.get(), gl_context);

  SDL_GL_SetSwapInterval(1); // Enable vsync

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
                                                        // io.ConfigViewportsNoAutoMerge = true;
                                                        // io.ConfigViewportsNoTaskBarIcon = true;

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

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForOpenGL(window.get(), gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts
  // and use ImGui::PushFont()/PopFont() to select them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font
  // among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your
  // application (e.g. use an assertion, or display an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when
  // calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
  // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality
  // font rendering.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to
  // write a double backslash \\ !
  // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the
  // "fonts/" folder. See Makefile.emscripten for details. io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  io.Fonts->AddFontFromFileTTF("assets/notepad_font/NotepadFont.ttf", 17.5f);
  // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL,
  // io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

  int originalX, originalY;
  int width, height;
  window.getPosition(&originalX, &originalY);
  window.getSize(&width, &height);

  App app;

  // Main loop
  bool done = false;
  auto lastUpdate = std::chrono::high_resolution_clock::now();
#ifdef __EMSCRIPTEN__
  // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen()
  // of the imgui.ini file. You may manually call LoadIniSettingsFromMemory() to load settings from
  // your own storage.
  io.IniFilename = NULL;
  EMSCRIPTEN_MAINLOOP_BEGIN
#else
  while (!done)
#endif
  {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants
    // to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application,
    // or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main
    // application, or clear/overwrite your copy of the keyboard data. Generally you may always pass
    // all inputs to dear imgui, and hide them from your application based on those two flags.
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
        done = true;
      else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
               event.window.windowID == SDL_GetWindowID(window.get()))
        done = true;
      else if (event.type == SDL_WINDOWEVENT)
      {
        if (event.window.event == SDL_WINDOWEVENT_MINIMIZED)
        {
          LOG("minimized", app.isMinimized);
          if (!app.isMinimized)
          {
            window.restore();
            window.restore();
            window.setPosition(10'000, originalY);
            window.setSize(width, height);
          }
          else
          {
            window.restore();
            window.restore();
            window.setPosition(originalX, originalY);
            window.setSize(width, height);
          }
          app.isMinimized = !app.isMinimized;
        }
        else
        {
          if (!app.isMinimized)
          {
            window.getPosition(&originalX, &originalY);
            window.getSize(&width, &height);
          }
        }
      }
    }

    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> diff = now - lastUpdate;
    lastUpdate = now;
    const auto dt = diff.count();
    app.tick(dt);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    app.renderUi(dt);

    ImGui::Render();

    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f); // Change the reference value (0.1f) to your desired threshold
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    const auto w = (int)io.DisplaySize.x == 0 ? width : (int)io.DisplaySize.x;
    const auto h = (int)io.DisplaySize.y == 0 ? height : (int)io.DisplaySize.y;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    app.render(dt);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it
    // easier to paste this code elsewhere.
    //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
      SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
      SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    window.glSwap();
    app.processIo();
  }
#ifdef __EMSCRIPTEN__
  EMSCRIPTEN_MAINLOOP_END;
#endif

  app.savePrj();

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(gl_context);

  return 0;
}
