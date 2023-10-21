#include "app.hpp"
#include "add-as-dialog.hpp"
#include "ai-mouth.hpp"
#include "anim-sprite.hpp"
#include "blink.hpp"
#include "bouncer.hpp"
#include "bouncer2.hpp"
#include "chat-v2.hpp"
#include "chat.hpp"
#include "eye-v2.hpp"
#include "eye.hpp"
#include "file-open.hpp"
#include "imgui-helpers.hpp"
#include "imgui-impl-opengl3.h"
#include "imgui-impl-sdl.h"
#include "input-dialog.hpp"
#include "message-dialog.hpp"
#include "mouth.hpp"
#include "preferences-dialog.hpp"
#include "prj-dialog.hpp"
#include "root.hpp"
#include "ui.hpp"
#include "version.hpp"
#include <SDL_opengl.h>
#include <fstream>
#include <log/log.hpp>

static auto getProjMat() -> glm::mat4
{
  GLfloat projMatData[16];
  glGetFloatv(GL_PROJECTION_MATRIX, projMatData);
  return glm::make_mat4(projMatData);
}

App::App(sdl::Window &aWindow, int argc, char *argv[])
  : window(aWindow),
    gl_context(SDL_GL_CreateContext(window.get().get())),
    lastUpdate(std::chrono::high_resolution_clock::now()),
    audioOut(preferences.audioOut),
    audioIn(uv, preferences.audioIn, wav2Visemes.sampleRate(), wav2Visemes.frameSize()),
    mouseTracking(uv),
    httpClient(uv),
    lib(preferences, uv, httpClient),
    selectIco(lib.queryTex("engine:select.png", true)),
    translateIco(lib.queryTex("engine:transalte.png", true)),
    scaleIco(lib.queryTex("engine:scale.png", true)),
    rotateIco(lib.queryTex("engine:rotate.png", true)),
    selectDisabledIco(lib.queryTex("engine:select-disabled.png", true)),
    translateDisabledIco(lib.queryTex("engine:transalte-disabled.png", true)),
    scaleDisabledIco(lib.queryTex("engine:scale-disabled.png", true)),
    rotateDisabledIco(lib.queryTex("engine:rotate-disabled.png", true)),
    hideIco(lib.queryTex("engine:eye-sprite.png", true)),
    showIco(lib.queryTex("engine:not-visable.png", true)),
    arrowN(lib.queryTex("engine:arrow-n-circle.png", true)),
    arrowE(lib.queryTex("engine:arrow-e-circle.png", true)),
    arrowS(lib.queryTex("engine:arrow-s-circle.png", true)),
    arrowW(lib.queryTex("engine:arrow-w-circle.png", true)),
    renderTimer(uv.createTimer()),
    renderIdle(uv.createIdle())
{
  SDL_GL_MakeCurrent(window.get().get(), gl_context);
  SDL_GL_SetSwapInterval(preferences.vsync ? 1 : 0);

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

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForOpenGL(window.get().get(), gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);

  auto &io = ImGui::GetIO();
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

  window.get().getPosition(&originalX, &originalY);
  window.get().getSize(&width, &height);

  LOG("sample rate:", wav2Visemes.sampleRate());
  LOG("frame size:", wav2Visemes.frameSize());
  audioIn.reg(wav2Visemes);
  saveFactory.reg<Bouncer>(
    [this](std::string) { return std::make_unique<Bouncer>(lib, undo, audioIn); });
  saveFactory.reg<Bouncer2>([this](std::string name) {
    return std::make_unique<Bouncer2>(lib, undo, audioIn, std::move(name));
  });
  saveFactory.reg<Root>([this](std::string) { return std::make_unique<Root>(lib, undo); });
  saveFactory.reg<SpriteSheetMouth>([this](std::string name) {
    return std::make_unique<SpriteSheetMouth>(wav2Visemes, lib, undo, std::move(name));
  });
  saveFactory.reg<ImageListMouth>([this](std::string name) {
    return std::make_unique<ImageListMouth>(wav2Visemes, lib, undo, std::move(name));
  });
  saveFactory.reg<AnimSprite>(
    [this](std::string name) { return std::make_unique<AnimSprite>(lib, undo, std::move(name)); });
  saveFactory.reg<Eye>([this](std::string name) {
    return std::make_unique<Eye>(mouseTracking, lib, undo, std::move(name));
  });
  saveFactory.reg<EyeV2>([this](std::string name) {
    return std::make_unique<EyeV2>(mouseTracking, lib, undo, std::move(name));
  });
  saveFactory.reg<Chat>([this](std::string name) {
    return std::make_unique<Chat>(lib, undo, uv, audioOut, std::move(name));
  });
  saveFactory.reg<ChatV2>([this](std::string name) {
    return std::make_unique<ChatV2>(lib, undo, uv, audioOut, std::move(name));
  });
  saveFactory.reg<AiMouth>([this](std::string name) {
    return std::make_unique<AiMouth>(lib, undo, audioIn, audioOut, wav2Visemes, std::move(name));
  });
  saveFactory.reg<SpriteSheetBlink>(
    [this](std::string name) { return std::make_unique<SpriteSheetBlink>(lib, undo, std::move(name)); });
  saveFactory.reg<ImageListBlink>(
    [this](std::string name) { return std::make_unique<ImageListBlink>(lib, undo, std::move(name)); });

  if (argc == 2)
  {
    std::filesystem::current_path(argv[1]);
    showUi = false;
    loadPrj();
  }
  setupRendering();
}

auto App::render(float dt) -> void
{
  if (!root)
  {
    glClearColor(0x45 / 255.f, 0x44 / 255.f, 0x7d / 255.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    return;
  }

  if (showUi && !isMinimized)
  {
    root->renderAll(dt, hovered, selected);

    if (selected)
    {
      const auto projMat = getProjMat();
      auto local = selected->localToScreen(projMat, selected->pivot());
      auto localX = selected->localToScreen(projMat, selected->pivot() + glm::vec2{1.f, 0.f});
      auto localY = selected->localToScreen(projMat, selected->pivot() + glm::vec2{0.f, 1.f});
      switch (editMode)
      {
      case EditMode::select: break;
      case EditMode::translate: {
        auto drawArrow = [&](const glm::vec2 &start, const glm::vec2 &dir, const glm::vec4 &color) {
          glColor4fv(glm::value_ptr(color));
          glVertex2fv(glm::value_ptr(start));

          auto end = start + 64.f * dir;
          glVertex2fv(glm::value_ptr(end));

          glm::vec2 ortho(-dir.y, dir.x);
          glm::vec2 arrowHeadBase = end - 15.f * dir;
          glVertex2fv(glm::value_ptr(end));
          glVertex2fv(glm::value_ptr(arrowHeadBase + 10.f * ortho));
          glVertex2fv(glm::value_ptr(end));
          glVertex2fv(glm::value_ptr(arrowHeadBase - 10.f * ortho));
        };

        glBegin(GL_LINES);
        drawArrow(local, glm::normalize(localX - local), glm::vec4(1.f, .0f, .0f, 1.f));
        drawArrow(local, glm::normalize(localY - local), glm::vec4(0.f, 1.f, .0f, 1.f));
        glEnd();
        break;
      }
      case EditMode::rotate: {
        auto radius = 64.f;
        const auto circlePoints = 100.0f;
        const auto increment = 2.0f * glm::pi<float>() / circlePoints;
        auto theta = 0.0f;
        glColor4f(0.f, 0.f, 1.f, 1.f);
        glBegin(GL_LINE_LOOP);
        for (auto i = 0.f; i < circlePoints; i++)
        {
          auto x = radius * cosf(theta) + local.x;
          auto y = radius * sinf(theta) + local.y;
          glVertex2f(x, y);
          theta += increment;
        }
        glEnd();
        glBegin(GL_LINES);
        glColor4f(1.f, .0f, .0f, 1.f);
        glVertex2fv(glm::value_ptr(local));
        glVertex2fv(glm::value_ptr(local + radius * glm::normalize(localX - local)));
        glColor4f(0.f, 1.f, .0f, 1.f);
        glVertex2fv(glm::value_ptr(local));
        glVertex2fv(glm::value_ptr(local + radius * glm::normalize(localY - local)));
        glEnd();
        break;
      }
      case EditMode::scale: {
        auto drawBox = [&](const glm::vec2 &start, const glm::vec2 &dir, const glm::vec4 &color) {
          glColor4fv(glm::value_ptr(color));
          glVertex2fv(glm::value_ptr(start));

          auto end = start + 64.f * dir;
          glVertex2fv(glm::value_ptr(end - 10.f * dir));

          glm::vec2 ortho(-dir.y, dir.x);
          glm::vec2 boxHeadBase = end - 10.f * dir;
          glm::vec2 boxHeadEnd = end + 10.f * dir;
          glVertex2fv(glm::value_ptr(boxHeadBase + 10.f * ortho));
          glVertex2fv(glm::value_ptr(boxHeadBase - 10.f * ortho));
          glVertex2fv(glm::value_ptr(boxHeadEnd + 10.f * ortho));
          glVertex2fv(glm::value_ptr(boxHeadEnd - 10.f * ortho));
          glVertex2fv(glm::value_ptr(boxHeadBase + 10.f * ortho));
          glVertex2fv(glm::value_ptr(boxHeadEnd + 10.f * ortho));
          glVertex2fv(glm::value_ptr(boxHeadBase - 10.f * ortho));
          glVertex2fv(glm::value_ptr(boxHeadEnd - 10.f * ortho));
        };

        glBegin(GL_LINES);
        drawBox(local, glm::normalize(localX - local), glm::vec4(1.f, .0f, .0f, 1.f));
        drawBox(local, glm::normalize(localY - local), glm::vec4(0.f, 1.f, .0f, 1.f));
        glEnd();
        break;
      }
      }
    }
  }
  else
    root->renderAll(dt, nullptr, nullptr);
}

auto App::renderUi(float /*dt*/) -> void
{
  if (!root)
  {
    if (!dialog)
      dialog = std::make_unique<PrjDialog>(lib, [this](bool) { loadPrj(); });
    if (!dialog->draw())
      dialog = nullptr;
    return;
  }

  ImGuiIO &io = ImGui::GetIO();
  ImGuiStyle &style = ImGui::GetStyle();
  if (isMinimized)
    return;
  if (!showUi)
  {
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
      style.Colors[ImGuiCol_WindowBg].w = .2f;
    auto showUiWindow = Ui::Window("##Show UI");
    const auto sz = 2 * ImGui::GetFontSize();
    if (Ui::btnImg("Show UI (U)", *showIco, sz, sz))
      showUi = true;
    if (ImGui::IsItemHovered())
      ImGui::SetTooltip("Show UI (U)");
    return;
  }

  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    style.Colors[ImGuiCol_WindowBg].w = .8f;
  if (auto mainMenu = Ui::MainMenuBar{})
  {
    if (auto fileMenu = Ui::Menu{"File"})
    {
      if (ImGui::MenuItem("Save", "Ctrl+S"))
        savePrj();
      ImGui::Separator();
      if (ImGui::MenuItem("Quit", "Alt+F4"))
        done = true;
    }
    if (auto editMenu = Ui::Menu{"Edit"})
    {
      {
        auto undoDisabled = Ui::Disabled(!undo.hasUndo());
        if (ImGui::MenuItem("Undo", "Ctrl+Z"))
          undo.undo();
      }
      {
        auto redoDisabled = Ui::Disabled(!undo.hasRedo());
        if (ImGui::MenuItem("Redo", "Ctrl+Y"))
          undo.redo();
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Select", "Shift+Q", editMode == EditMode::select))
        editMode = EditMode::select;
      if (ImGui::MenuItem("Translate", "Shift+W", editMode == EditMode::translate))
        editMode = EditMode::translate;
      if (ImGui::MenuItem("Rotate", "Shift+E", editMode == EditMode::rotate))
        editMode = EditMode::rotate;
      if (ImGui::MenuItem("Scale", "Shift+R", editMode == EditMode::scale))
        editMode = EditMode::scale;
      ImGui::Separator();
      if (ImGui::MenuItem("Add Sprite..."))
        dialog =
          std::make_unique<FileOpen>(lib, "Add Sprite Dialog", [this](bool r, const auto &filePath) {
            if (r)
              addNode(AnimSprite::className, filePath.string());
          });
      if (ImGui::MenuItem("Add Sprite Sheet Mouth..."))
        dialog = std::make_unique<FileOpen>(
          lib, "Add Sprite Sheet Mouth Dialog", [this](bool r, const auto &filePath) {
            if (r)
              addNode(SpriteSheetMouth::className, filePath.string());
          });
      if (ImGui::MenuItem("Add Image List Mouth..."))
        dialog =
          std::make_unique<InputDialog>("Enter Node Name", "Mouth", [this](bool r, const auto &input) {
            if (r)
              addNode(ImageListMouth::className, input);
          });
      if (ImGui::MenuItem("Add Eye..."))
        dialog = std::make_unique<FileOpen>(lib, "Add Eye Dialog", [this](bool r, const auto &filePath) {
          if (r)
            addNode(EyeV2::className, filePath.string());
        });
      if (ImGui::MenuItem("Add Sprite Sheet Blink..."))
        dialog = std::make_unique<FileOpen>(
          lib, "Add Sprite Sheet Blink Dialog", [this](bool r, const auto &filePath) {
            if (r)
              addNode(SpriteSheetBlink::className, filePath.string());
          });
      if (ImGui::MenuItem("Add Image List Blink..."))
        dialog =
          std::make_unique<InputDialog>("Enter Node Name", "Blink", [this](bool r, const auto &input) {
            if (r)
              addNode(ImageListBlink::className, input);
          });
      if (ImGui::MenuItem("Add Twitch Chat..."))
        dialog = std::make_unique<InputDialog>(
          "Enter Twitch Channel Name", "mika314", [this](bool r, const auto &channel) {
            if (r)
              addNode(ChatV2::className, channel);
          });
      if (ImGui::MenuItem("Add Bouncer"))
        addNode(Bouncer2::className, "bouncer");
      if (ImGui::MenuItem("Add AI Mouth..."))
        dialog =
          std::make_unique<FileOpen>(lib, "Add AI Mouth Dialog", [this](bool r, const auto &filePath) {
            if (r)
              addNode(AiMouth::className, filePath.string());
          });
      ImGui::Separator();
      {
        auto delDisabled = Ui::Disabled(!selected);
        if (ImGui::MenuItem("Toggle Visiblity", "H"))
          if (selected)
            undo.record([isVisible = selected->visible, this]() { selected->visible = !isVisible; },
                        [isVisible = selected->visible, this]() { selected->visible = isVisible; });
      }
      {
        auto delDisabled = Ui::Disabled(!selected);
        if (ImGui::MenuItem("Delete", "Del"))
          if (selected)
            Node::del(&selected);
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Preferences..."))
        dialog = std::make_unique<PreferencesDialog>(preferences, audioOut, audioIn, [this](bool r) {
          if (!r)
            return;
          lib.flush();
          setupRendering();
        });
    }
  }

  if (dialog)
    if (!dialog->draw())
      dialog = nullptr;
  {
    auto outlinerWindow = Ui::Window("Outliner");
    {
      {
        const auto sz = 2 * ImGui::GetFontSize();
        if (Ui::btnImg("Hide UI (U)", *hideIco, sz, sz))
          showUi = false;
        if (ImGui::IsItemHovered())
          ImGui::SetTooltip("Hide UI (U)");
        ImGui::SameLine();
        if (Ui::btnImg("Select", editMode == EditMode::select ? *selectIco : *selectDisabledIco, sz, sz))
          editMode = EditMode::select;
        if (ImGui::IsItemHovered())
          ImGui::SetTooltip("Select (Shift+Q)");
        ImGui::SameLine();
        if (Ui::btnImg("Translate",
                       editMode == EditMode::translate ? *translateIco : *translateDisabledIco,
                       sz,
                       sz))
          editMode = EditMode::translate;
        if (ImGui::IsItemHovered())
          ImGui::SetTooltip("Translate (Shift+W)");
        ImGui::SameLine();
        if (Ui::btnImg("Rotate", editMode == EditMode::rotate ? *rotateIco : *rotateDisabledIco, sz, sz))
          editMode = EditMode::rotate;
        if (ImGui::IsItemHovered())
          ImGui::SetTooltip("Rotate (Shift+E)");
        ImGui::SameLine();
        if (Ui::btnImg("Scale", editMode == EditMode::scale ? *scaleIco : *scaleDisabledIco, sz, sz))
          editMode = EditMode::scale;
        if (ImGui::IsItemHovered())
          ImGui::SetTooltip("Scale (Shift+R)");
      }

      auto hierarchyButtonsDisabled = Ui::Disabled(!selected);
      const auto sz = ImGui::GetFontSize();
      if (Ui::btnImg("<", *arrowW, sz, sz))
        selected->unparent();
      if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Unparent");
      ImGui::SameLine();
      if (Ui::btnImg("^", *arrowN, sz, sz))
        selected->moveUp();
      if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Move up");
      ImGui::SameLine();
      if (Ui::btnImg("V", *arrowS, sz, sz))
        selected->moveDown();
      if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Move down");
      ImGui::SameLine();
      if (Ui::btnImg(">", *arrowE, sz, sz))
        selected->parentWithBellow();
      if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Parent with below");
    }
    renderTree(*root);
    ImGui::TextF("{:3f} ms/frame ({:1f} FPS)", 1000.0f / io.Framerate, io.Framerate);
  }
  {
    auto detailsWindow = Ui::Window("Details");
    if (selected)
      if (auto detailsTable = Ui::Table{"Details", 2, ImGuiTableFlags_SizingStretchProp})
      {
        ImGui::TableSetupColumn("Property       ", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        selected->renderUi();
      }
  }
  for (auto &action : postponedActions)
    action();
  postponedActions.clear();
}

auto App::processIo() -> void
{
  if (!root)
    return;
  // Check if ImGui did not process any user input
  ImGuiIO &io = ImGui::GetIO();
  if (!io.WantCaptureMouse)
  {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
      if (!selected || selected->editMode() == Node::EditMode::select)
      {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        const auto projMat = getProjMat();
        auto newSelected = root->nodeUnder(projMat, glm::vec2{1.f * mouseX, 1.f * mouseY});
        if (newSelected != selected)
          undo.record([newSelected, this]() { selected = newSelected; },
                      [oldSelected = selected, this]() { selected = oldSelected; });
      }
      else
      {
        if (selected)
        {
          selected->commit();
          isNodeDragging = false;
        }
      }
    }
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
      cancel();

    if (selected)
    {
      if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
      {
        if (!isNodeDragging)
        {
          auto dragDelta = ImGui::GetMouseDragDelta(0);
          if (std::abs(dragDelta.x) > 5 || std::abs(dragDelta.y) > 5)
          {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            switch (editMode)
            {
            case EditMode::select: break;
            case EditMode::translate:
              isNodeDragging = true;
              selected->translateStart(glm::vec2{1.f * mouseX, 1.f * mouseY});
              break;
            case EditMode::rotate:
              isNodeDragging = true;
              selected->rotStart(glm::vec2{1.f * mouseX, 1.f * mouseY});
              break;
            case EditMode::scale:
              isNodeDragging = true;
              selected->scaleStart(glm::vec2{1.f * mouseX, 1.f * mouseY});
              break;
            }
          }
        }
      }
      else
      {
        if (isNodeDragging)
        {
          selected->commit();
          isNodeDragging = false;
        }
      }
    }
  }
  if (!io.WantCaptureKeyboard || !io.WantCaptureMouse)
  {
    if (selected && !io.KeyCtrl && !io.KeyShift && !io.KeyAlt && !io.KeySuper)
    {
      if (ImGui::IsKeyPressed(ImGuiKey_G))
      {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        selected->translateStart(glm::vec2{1.f * mouseX, 1.f * mouseY});
      }

      if (ImGui::IsKeyPressed(ImGuiKey_S))
      {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        selected->scaleStart(glm::vec2{1.f * mouseX, 1.f * mouseY});
      }

      if (ImGui::IsKeyPressed(ImGuiKey_R))
      {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        selected->rotStart(glm::vec2{1.f * mouseX, 1.f * mouseY});
      }
      if (ImGui::IsKeyPressed(ImGuiKey_X) || ImGui::IsKeyPressed(ImGuiKey_Delete))
        Node::del(&selected);
      if (ImGui::IsKeyPressed(ImGuiKey_H))
        undo.record([isVisible = selected->visible, this]() { selected->visible = !isVisible; },
                    [isVisible = selected->visible, this]() { selected->visible = isVisible; });
      if (ImGui::IsKeyPressed(ImGuiKey_U))
        showUi = !showUi;
      if (ImGui::IsKeyPressed(ImGuiKey_D))
      {
        if (selected)
        {
          OStrm os;
          selected->saveAll(os);
          const auto s = os.str();
          IStrm is(s.data(), s.data() + s.size());
          std::string className;
          std::string name;
          ::deser(is, className);
          ::deser(is, name);
          LOG(className, name);
          auto n = std::shared_ptr{saveFactory.ctor(className, name)};
          n->loadAll(saveFactory, is);
          undo.record(
            [n, parent = selected->parent(), this]() {
              selected = n.get();
              parent->addChild(std::move(n));
            },
            [n, oldSelected = selected, this]() {
              Node::delNoUndo(*n);
              selected = oldSelected;
            });
          int mouseX, mouseY;
          SDL_GetMouseState(&mouseX, &mouseY);
          selected->translateStart(glm::vec2{1.f * mouseX, 1.f * mouseY});
        }
      }
      if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        cancel();
    }
    if (!io.KeyCtrl && io.KeyShift && !io.KeyAlt && !io.KeySuper)
    {
      if (ImGui::IsKeyPressed(ImGuiKey_Q))
        editMode = EditMode::select;
      if (ImGui::IsKeyPressed(ImGuiKey_W))
        editMode = EditMode::translate;
      if (ImGui::IsKeyPressed(ImGuiKey_E))
        editMode = EditMode::rotate;
      if (ImGui::IsKeyPressed(ImGuiKey_R))
        editMode = EditMode::scale;
    }
    if (io.KeyCtrl && !io.KeyShift && !io.KeyAlt && !io.KeySuper)
    {
      if (ImGui::IsKeyPressed(ImGuiKey_S))
        savePrj();
    }
  }
  if (io.KeyCtrl && !io.KeyShift && !io.KeyAlt && !io.KeySuper && ImGui::IsKeyPressed(ImGuiKey_Z))
    undo.undo();
  if (io.KeyCtrl && !io.KeyShift && !io.KeyAlt && !io.KeySuper && ImGui::IsKeyPressed(ImGuiKey_Y))
    undo.redo();
}

auto App::cancel() -> void
{
  if (!selected)
    return;

  isNodeDragging = false;
  selected->cancel();
}

auto App::renderTree(Node &v) -> void
{
  ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                 ImGuiTreeNodeFlags_SpanAvailWidth;
  ImGuiTreeNodeFlags nodeFlags = baseFlags;

  const auto nm = v.getName();
  if (selected == &v)
    nodeFlags |= ImGuiTreeNodeFlags_Selected;
  auto dragAndDrop = [&]() {
    if (auto source = Ui::DragDropSource{})
    {
      auto pNode = &v;
      ImGui::SetDragDropPayload("_TREENODE", &pNode, sizeof pNode);
      ImGui::Text("Hold Ctrl to parent");
    }
    auto payload = ImGui::GetDragDropPayload();
    if (!payload)
      return;
    if (!payload->IsDataType("_TREENODE"))
      return;
    auto srcNode = *static_cast<Node **>(payload->Data);
    if (!srcNode)
      return;
    ImGuiIO &io = ImGui::GetIO();
    const auto ctrl = io.KeyCtrl && !io.KeyShift && !io.KeyAlt && !io.KeySuper;
    if (&v == root.get() && !ctrl)
      return;
    for (auto p = v.parent(); p != nullptr; p = p->parent())
      if (p == srcNode)
        return;
    if (!Ui::DragDropTarget{})
      return;
    if (ImGui::AcceptDragDropPayload("_TREENODE"))
    {
      if (!io.KeyCtrl && !io.KeyShift && !io.KeyAlt && !io.KeySuper)
      {
        LOG(srcNode->getName(), "->", v.getName());
        postponedActions.emplace_back([srcNode, &v]() { srcNode->placeBellow(v); });
      }
      if (ctrl)
      {
        LOG("Parent", srcNode->getName(), "->", v.getName());
        postponedActions.emplace_back([srcNode, &v]() { srcNode->parentWith(v); });
      }
    }
  };
  const auto &nodes = v.getNodes();
  const auto sz = ImGui::GetFontSize();
  std::ostringstream ss;
  ss << "##" << &v;
  if (!nodes.empty())
  {
    if (Ui::btnImg(ss.str().c_str(), v.visible ? *hideIco : *showIco, sz, sz))
      undo.record([&v, newVisibility = !v.visible]() { v.visible = newVisibility; },
                  [&v, oldVisibility = v.visible]() { v.visible = oldVisibility; });

    ImGui::SameLine();
    nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
    const auto nodeOpen = ImGui::TreeNodeEx(&v, nodeFlags, "%s", nm.c_str());
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
      undo.record([&v, this]() { selected = &v; },
                  [oldSelected = selected, this]() { selected = oldSelected; });
    dragAndDrop();
    if (nodeOpen)
    {
      for (const auto &n : nodes)
        renderTree(*n);
      ImGui::TreePop();
    }
  }
  else
  {
    if (Ui::btnImg(ss.str().c_str(), v.visible ? *hideIco : *showIco, sz, sz))
      undo.record([&v, newVisibility = !v.visible]() { v.visible = newVisibility; },
                  [&v, oldVisibility = v.visible]() { v.visible = oldVisibility; });
    ImGui::SameLine();
    nodeFlags |=
      ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
    ImGui::TreeNodeEx(&v, nodeFlags, "%s", nm.c_str());
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
      undo.record([&v, this]() { selected = &v; },
                  [oldSelected = selected, this]() { selected = oldSelected; });
    dragAndDrop();
  }
}

auto App::loadPrj() -> void
{
  ImGui::LoadIniSettingsFromDisk("imgui.ini");

  std::ifstream st("prj.tpp", std::ofstream::binary);
  if (!st)
  {
    root = std::make_unique<Root>(lib, undo);
    LOG("Create new project");
    return;
  }

  std::ostringstream buffer;
  buffer << st.rdbuf();

  auto buf = buffer.str();

  IStrm strm(buf.data(), buf.data() + buf.size());

  uint32_t v;
  ::deser(strm, v);
  if (v != saveVersion())
  {
    root = std::make_unique<Root>(lib, undo);
    LOG("Version mismatch expected:", saveVersion(), ", received:", v);
    return;
  }

  std::string className;
  std::string name;
  ::deser(strm, className);
  ::deser(strm, name);
  LOG(className, name);
  root = saveFactory.ctor(className, name);
  root->loadAll(saveFactory, strm);
}

auto App::savePrj() -> void
{
  if (!root)
    return;
  OStrm strm;
  ::ser(strm, saveVersion());
  root->saveAll(strm);
  std::ofstream st("prj.tpp", std::ofstream::binary);
  st.write(strm.str().data(), strm.str().size());
}

auto App::addNode(const std::string &class_, const std::string &name) -> void
{
  try
  {
    auto node = std::shared_ptr{saveFactory.ctor(class_, name)};
    auto oldSelected = selected;
    undo.record(
      [node, parent = (oldSelected ? oldSelected : root.get()), this]() {
        selected = node.get();
        parent->addChild(std::move(node));
      },
      [node, oldSelected, this]() {
        Node::delNoUndo(*selected);
        selected = oldSelected;
      });
  }
  catch (const std::runtime_error &e)
  {
    postponedActions.emplace_back(
      [&]() { dialog = std::make_unique<MessageDialog>("Error", e.what()); });
    LOG(e.what());
  }
};

auto App::droppedFile(std::string droppedFile) -> void

{
  dialog =
    std::make_unique<AddAsDialog>(droppedFile, [this, droppedFile](bool r, AddAsDialog::NodeType t) {
      if (!r)
        return;
      switch (t)
      {
      case AddAsDialog::NodeType::sprite: addNode(AnimSprite::className, droppedFile); break;
      case AddAsDialog::NodeType::mouth: addNode(SpriteSheetMouth::className, droppedFile); break;
      case AddAsDialog::NodeType::eye: addNode(EyeV2::className, droppedFile); break;
      case AddAsDialog::NodeType::aiMouth: addNode(AiMouth::className, droppedFile); break;
      }
    });
}

auto App::tick() -> void
{
  uv.tick();
}

App::~App()
{
  savePrj();

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  SDL_GL_DeleteContext(gl_context);
}

auto App::sdlEventsAndRender() -> void
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
    switch (event.type)
    {
    case SDL_QUIT: done = true; break;
    case SDL_WINDOWEVENT:
      if (event.window.event == SDL_WINDOWEVENT_CLOSE &&
          event.window.windowID == SDL_GetWindowID(window.get().get()))
        done = true;
      else if (event.window.event == SDL_WINDOWEVENT_MINIMIZED)
      {
        if (!isMinimized)
        {
          window.get().restore();
          window.get().restore();
          window.get().setPosition(10'000, originalY);
          window.get().setSize(width, height);
        }
        else
        {
          window.get().restore();
          window.get().restore();
          window.get().setPosition(originalX, originalY);
          window.get().setSize(width, height);
        }
        isMinimized = !isMinimized;
      }
      else
      {
        if (!isMinimized)
        {
          window.get().getPosition(&originalX, &originalY);
          window.get().getSize(&width, &height);
        }
      }
      break;
    case SDL_DROPFILE: {
      auto file = event.drop.file;
      LOG("dropped file", file);
      droppedFile(file);
      SDL_free(file);
      break;
    }
    case SDL_MOUSEMOTION: {
      if (!root)
        break;
      const auto projMat = getProjMat();
      const auto mouseX = event.motion.x;
      const auto mouseY = event.motion.y;
      hovered = nullptr;
      const auto mousePos = glm::vec2{1.f * mouseX, 1.f * mouseY};
      if (!selected || selected->editMode() == Node::EditMode::select)
        hovered = root->nodeUnder(projMat, mousePos);
      else
      {
        if (selected)
          selected->update(projMat, mousePos);
      }
      break;
    }
    }
  }

  auto now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> diff = now - lastUpdate;
  lastUpdate = now;
  const auto dt = diff.count();

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  renderUi(dt);

  ImGui::Render();

  glEnable(GL_BLEND);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.1f); // Change the reference value (0.1f) to your desired threshold
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  ImGuiIO &io = ImGui::GetIO();
  const auto w = (int)io.DisplaySize.x == 0 ? width : (int)io.DisplaySize.x;
  const auto h = (int)io.DisplaySize.y == 0 ? height : (int)io.DisplaySize.y;
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, w, 0, h, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  render(dt);

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

  window.get().glSwap();
  processIo();
}

auto App::setupRendering() -> void
{
  renderTimer.stop();
  renderIdle.stop();
  const auto fps = preferences.fps;
  if (fps == 0)
    renderIdle.start([this]() { sdlEventsAndRender(); });
  else
    renderTimer.start([this]() { sdlEventsAndRender(); }, 0, 1'000 / fps);
}
