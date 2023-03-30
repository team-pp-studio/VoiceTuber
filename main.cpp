// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal
// graphics context creation, etc.) If you are new to Dear ImGui, read documentation from the docs/
// folder + read the top of imgui.cpp. Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "audio-capture.hpp"
#include "bouncer.hpp"
#include "imgui-impl-opengl3.h"
#include "imgui-impl-sdl.h"
#include "sprite.hpp"
#include "wav-2-visemes.hpp"
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
  Viseme curViseme;
  auto wav2Visemes = Wav2Visemes{[&curViseme](Viseme val) { curViseme = val; }};
  LOG("sample rate:", wav2Visemes.sampleRate());
  LOG("frame size:", wav2Visemes.frameSize());
  auto audioCapture = AudioCapture{wav2Visemes.sampleRate(), wav2Visemes.frameSize()};
  audioCapture.reg(wav2Visemes);

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
    (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

  auto window =
    sdl::Window{"PNGProject", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags};
  SDL_GLContext gl_context = SDL_GL_CreateContext(window.get());
  SDL_GL_MakeCurrent(window.get(), gl_context);
  SDL_GL_SetSwapInterval(1); // Enable vsync

  Bouncer root;
  audioCapture.reg(root);

  Sprite sprite1("visemes.png");
  sprite1.cols = 3;
  sprite1.rows = 5;
  sprite1.loc = {.0f, 100.f};
  sprite1.pivot = {512.f, 600.f - 293.f};
  Sprite sprite2("visemes.png");
  sprite2.cols = 3;
  sprite2.rows = 5;
  sprite2.loc = {512.f, 100.f};
  sprite2.pivot = {512.f, 600.f - 293.f};
  root.nodes.push_back(sprite1);
  root.nodes.push_back(sprite2);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
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
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
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
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL,
  // io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

  // Our state
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Main loop
  bool done = false;
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
      if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
          event.window.windowID == SDL_GetWindowID(window.get()))
        done = true;
    }

    audioCapture.tick();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named
    // window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Z-order");

      ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)

      ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

      if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when
                                   // edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);
      switch (curViseme)
      {
      case Viseme::sil: ImGui::Text("Viseme: sil"); break;
      case Viseme::PP: ImGui::Text("Viseme: PP"); break;
      case Viseme::FF: ImGui::Text("Viseme: FF"); break;
      case Viseme::TH: ImGui::Text("Viseme: TH"); break;
      case Viseme::DD: ImGui::Text("Viseme: DD"); break;
      case Viseme::kk: ImGui::Text("Viseme: kk"); break;
      case Viseme::CH: ImGui::Text("Viseme: CH"); break;
      case Viseme::SS: ImGui::Text("Viseme: SS"); break;
      case Viseme::nn: ImGui::Text("Viseme: nn"); break;
      case Viseme::RR: ImGui::Text("Viseme: RR"); break;
      case Viseme::aa: ImGui::Text("Viseme: aa"); break;
      case Viseme::E: ImGui::Text("Viseme: E"); break;
      case Viseme::I: ImGui::Text("Viseme: I"); break;
      case Viseme::O: ImGui::Text("Viseme: O"); break;
      case Viseme::U: ImGui::Text("Viseme: U"); break;
      }

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
      ImGui::End();
    }
    {
      ImGui::Begin("Details");
      ImGui::End();
    }

    // ImGui rendering
    ImGui::Render();

    // Enable blending
    glEnable(GL_BLEND);
    // Set blending function
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    const auto w = (int)io.DisplaySize.x;
    const auto h = (int)io.DisplaySize.y;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(clear_color.x * clear_color.w,
                 clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w,
                 clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor4f(1.f, 1.f, 1.f, 1.f);
    sprite1.viseme = curViseme;
    sprite2.rot -= 1.f;
    sprite2.viseme = curViseme;
    sprite2.scale = {sinf(.1f * sprite2.rot), sinf(.1f * sprite2.rot)};
    root.renderAll();

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
  }
#ifdef __EMSCRIPTEN__
  EMSCRIPTEN_MAINLOOP_END;
#endif

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(gl_context);

  return 0;
}
