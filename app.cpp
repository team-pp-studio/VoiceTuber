#include "app.hpp"
#include "anim-sprite.hpp"
#include "bouncer.hpp"
#include "bouncer2.hpp"
#include "channel-dialog.hpp"
#include "chat.hpp"
#include "eye.hpp"
#include "file-open.hpp"
#include "message-dialog.hpp"
#include "mouth.hpp"
#include "preferences-dialog.hpp"
#include "prj-dialog.hpp"
#include "root.hpp"
#include "ui.hpp"
#include <SDL_opengl.h>
#include <fstream>
#include <log/log.hpp>

static auto getProjMat() -> glm::mat4
{
  GLfloat projMatData[16];
  glGetFloatv(GL_PROJECTION_MATRIX, projMatData);
  return glm::make_mat4(projMatData);
}

App::App()
  : audioInput(preferences.inputAudio, wav2Visemes.sampleRate(), wav2Visemes.frameSize()),
    lib(preferences),
    selectIco(lib.queryTex("engine:select.png", true)),
    translateIco(lib.queryTex("engine:transalte.png", true)),
    scaleIco(lib.queryTex("engine:scale.png", true)),
    rotateIco(lib.queryTex("engine:rotate.png", true)),
    selectDisabledIco(lib.queryTex("engine:select-disabled.png", true)),
    translateDisabledIco(lib.queryTex("engine:transalte-disabled.png", true)),
    scaleDisabledIco(lib.queryTex("engine:scale-disabled.png", true)),
    rotateDisabledIco(lib.queryTex("engine:rotate-disabled.png", true)),
    hideUiIco(lib.queryTex("engine:eye-sprite.png", true)),
    showUiIco(lib.queryTex("engine:not-visable.png", true))
{
  LOG("sample rate:", wav2Visemes.sampleRate());
  LOG("frame size:", wav2Visemes.frameSize());
  audioInput.reg(wav2Visemes);
  saveFactory.reg<Bouncer>(
    [this](std::string) { return std::make_unique<Bouncer>(lib, undo, audioInput); });
  saveFactory.reg<Bouncer2>([this](std::string name) {
    return std::make_unique<Bouncer2>(lib, undo, audioInput, std::move(name));
  });
  saveFactory.reg<Root>([this](std::string) { return std::make_unique<Root>(lib, undo); });
  saveFactory.reg<Mouth>([this](std::string name) {
    return std::make_unique<Mouth>(wav2Visemes, lib, undo, std::move(name));
  });
  saveFactory.reg<AnimSprite>(
    [this](std::string name) { return std::make_unique<AnimSprite>(lib, undo, std::move(name)); });
  saveFactory.reg<Eye>([this](std::string name) {
    return std::make_unique<Eye>(mouseTracking, lib, undo, std::move(name));
  });
  saveFactory.reg<Chat>(
    [this](std::string name) { return std::make_unique<Chat>(lib, undo, uv, std::move(name)); });
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
        for (int i = 0; i < circlePoints; i++)
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
    if (Ui::btnImg("Show UI", *showUiIco, sz, sz))
      showUi = true;
    if (ImGui::IsItemHovered())
      ImGui::SetTooltip("Show UI");
    return;
  }

  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    style.Colors[ImGuiCol_WindowBg].w = .8f;
  if (auto mainMenu = Ui::MainMenuBar{})
  {
    if (auto fileMenu = Ui::Menu{"File"})
    {
      if (ImGui::MenuItem("Save"))
        savePrj();
      ImGui::Separator();
      if (ImGui::MenuItem("Quit"))
        done = true;
    }
    if (auto editMenu = Ui::Menu{"Edit"})
    {
      {
        auto undoDisabled = Ui::Disabled(!undo.hasUndo());
        if (auto undoMenu = ImGui::MenuItem("Undo", "CTRL+Z"))
          undo.undo();
      }
      {
        auto redoDisabled = Ui::Disabled(!undo.hasRedo());
        if (auto redoMenu = ImGui::MenuItem("Redo", "CTRL+Y"))
          undo.redo();
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Select", "", editMode == EditMode::select))
        editMode = EditMode::select;
      if (ImGui::MenuItem("Translate", "", editMode == EditMode::translate))
        editMode = EditMode::translate;
      if (ImGui::MenuItem("Rotate", "", editMode == EditMode::rotate))
        editMode = EditMode::rotate;
      if (ImGui::MenuItem("Scale", "", editMode == EditMode::scale))
        editMode = EditMode::scale;
      ImGui::Separator();
      if (ImGui::MenuItem("Add Mouth..."))
        dialog =
          std::make_unique<FileOpen>(lib, "Add Mouth Dialog", [this](bool r, const auto &filePath) {
            if (r)
              addNode(Mouth::className, filePath.string());
          });
      if (ImGui::MenuItem("Add Eye..."))
        dialog = std::make_unique<FileOpen>(lib, "Add Eye Dialog", [this](bool r, const auto &filePath) {
          if (r)
            addNode(Eye::className, filePath.string());
        });

      if (ImGui::MenuItem("Add Sprite..."))
        dialog =
          std::make_unique<FileOpen>(lib, "Add Sprite Dialog", [this](bool r, const auto &filePath) {
            if (r)
              addNode(AnimSprite::className, filePath.string());
          });

      if (ImGui::MenuItem("Add Twitch Chat..."))
        dialog = std::make_unique<ChannelDialog>("mika314", [this](bool r, const auto &channel) {
          if (r)
            addNode(Chat::className, channel);
        });
      if (ImGui::MenuItem("Add Bouncer"))
        addNode(Bouncer2::className, "bouncer");
      ImGui::Separator();
      if (ImGui::MenuItem("Preferences..."))
        dialog =
          std::make_unique<PreferencesDialog>(preferences, audioOutput, audioInput, [this](bool r) {
            if (r)
              lib.flush();
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
        if (Ui::btnImg("Hide UI", *hideUiIco, sz, sz))
          showUi = false;
        if (ImGui::IsItemHovered())
          ImGui::SetTooltip("Hide UI");
        ImGui::SameLine();
        if (Ui::btnImg("Select", editMode == EditMode::select ? *selectIco : *selectDisabledIco, sz, sz))
          editMode = EditMode::select;
        if (ImGui::IsItemHovered())
          ImGui::SetTooltip("Select (Shift-Q)");
        ImGui::SameLine();
        if (Ui::btnImg("Translate",
                       editMode == EditMode::translate ? *translateIco : *translateDisabledIco,
                       sz,
                       sz))
          editMode = EditMode::translate;
        if (ImGui::IsItemHovered())
          ImGui::SetTooltip("Translate (Shift-W)");
        ImGui::SameLine();
        if (Ui::btnImg("Rotate", editMode == EditMode::rotate ? *rotateIco : *rotateDisabledIco, sz, sz))
          editMode = EditMode::rotate;
        if (ImGui::IsItemHovered())
          ImGui::SetTooltip("Rotate (Shift-E)");
        ImGui::SameLine();
        if (Ui::btnImg("Scale", editMode == EditMode::scale ? *scaleIco : *scaleDisabledIco, sz, sz))
          editMode = EditMode::scale;
        if (ImGui::IsItemHovered())
          ImGui::SetTooltip("Scale (Shift-R)");
      }

      auto hierarchyButtonsDisabled = Ui::Disabled(!selected);
      if (ImGui::Button("<"))
        selected->unparent();
      if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Unparent");
      ImGui::SameLine();
      if (ImGui::Button("^"))
        selected->moveUp();
      if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Move up");
      ImGui::SameLine();
      if (ImGui::Button("V"))
        selected->moveDown();
      if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Move down");
      ImGui::SameLine();
      if (ImGui::Button(">"))
        selected->parentWithBellow();
      if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Parent with below");
    }
    renderTree(*root);
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
  }
  {
    auto detailsWindpw = Ui::Window("Details");
    if (selected)
      if (auto detailsTable = Ui::Table{"Details", 2, ImGuiTableFlags_SizingStretchProp})
      {
        ImGui::TableSetupColumn("Property     ", ImGuiTableColumnFlags_WidthFixed);
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

auto App::tick(float /*dt*/) -> void
{
  audioInput.tick();
  uv.tick();

  if (!root)
    return;

  const auto projMat = getProjMat();
  int mouseX, mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);
  hovered = nullptr;
  const auto mousePos = glm::vec2{1.f * mouseX, 1.f * mouseY};
  if (!selected || selected->editMode() == Node::EditMode::select)
    hovered = root->nodeUnder(projMat, mousePos);
  else
  {
    if (selected)
      selected->update(projMat, mousePos);
  }
  mouseTracking.tick();
}

auto App::renderTree(Node &v) -> void
{
  ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                 ImGuiTreeNodeFlags_SpanAvailWidth;
  ImGuiTreeNodeFlags nodeFlags = baseFlags;

  const auto nm = v.getName();
  if (selected == &v)
    nodeFlags |= ImGuiTreeNodeFlags_Selected;
  const auto &nodes = v.getNodes();
  if (!nodes.empty())
  {
    nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
    const auto nodeOpen = ImGui::TreeNodeEx(&v, nodeFlags, "%s", nm.c_str());
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
      undo.record([&v, this]() { selected = &v; },
                  [oldSelected = selected, this]() { selected = oldSelected; });
    if (nodeOpen)
    {
      for (const auto &n : nodes)
        renderTree(*n);
      ImGui::TreePop();
    }
  }
  else
  {
    nodeFlags |=
      ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
    ImGui::TreeNodeEx(&v, nodeFlags, "%s", nm.c_str());
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
      undo.record([&v, this]() { selected = &v; },
                  [oldSelected = selected, this]() { selected = oldSelected; });
  }
}

static const auto ver = uint32_t{2};

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
  if (v != ver)
  {
    root = std::make_unique<Root>(lib, undo);
    LOG("Version mismatch expected:", ver, ", received:", v);
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
  ::ser(strm, ver);
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
