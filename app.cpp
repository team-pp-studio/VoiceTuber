#include "app.hpp"
#include "anim-sprite.hpp"
#include "bouncer.hpp"
#include "channel-dialog.hpp"
#include "chat.hpp"
#include "eye.hpp"
#include "file-open.hpp"
#include "message-dialog.hpp"
#include "mouth.hpp"
#include "preferences-dialog.hpp"
#include "prj-dialog.hpp"
#include <SDL_opengl.h>
#include <fstream>
#include <log/log.hpp>

static auto getProjMat() -> glm::mat4
{
  GLfloat projMatData[16];
  glGetFloatv(GL_PROJECTION_MATRIX, projMatData);
  return glm::make_mat4(projMatData);
}

App::App() : audioCapture(wav2Visemes.sampleRate(), wav2Visemes.frameSize()), lib(preferences)
{
  LOG("sample rate:", wav2Visemes.sampleRate());
  LOG("frame size:", wav2Visemes.frameSize());
  audioCapture.reg(wav2Visemes);
  saveFactory.reg<Bouncer>([this](std::string) { return std::make_unique<Bouncer>(audioCapture); });
  saveFactory.reg<Mouth>(
    [this](std::string name) { return std::make_unique<Mouth>(wav2Visemes, lib, std::move(name)); });
  saveFactory.reg<AnimSprite>(
    [this](std::string name) { return std::make_unique<AnimSprite>(lib, std::move(name)); });
  saveFactory.reg<Eye>(
    [this](std::string name) { return std::make_unique<Eye>(mouseTracking, lib, std::move(name)); });
  saveFactory.reg<Chat>(
    [this](std::string name) { return std::make_unique<Chat>(lib, std::move(name)); });
}

auto App::render(float dt) -> void
{
  if (!root)
  {
    glClearColor(0x45 / 255.f, 0x44 / 255.f, 0x7d / 255.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    return;
  }

  if (showUi)
    root->renderAll(dt, hovered, selected);
  else
    root->renderAll(dt, nullptr, nullptr);
}

auto App::renderUi(float /*dt*/) -> void
{
  if (!root)
  {
    if (!dialog)
      dialog = std::make_unique<PrjDialog>([this](bool) { loadPrj(); });
    if (!dialog->draw())
      dialog = nullptr;
    return;
  }

  ImGuiIO &io = ImGui::GetIO();
  ImGuiStyle &style = ImGui::GetStyle();
  if (!showUi)
  {
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
      style.Colors[ImGuiCol_WindowBg].w = .2f;
    ImGui::Begin("##Show UI");
    if (ImGui::Button("Show UI"))
      showUi = true;
    ImGui::End();
    return;
  }

  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    style.Colors[ImGuiCol_WindowBg].w = .8f;
  if (ImGui::BeginMainMenuBar())
  {
    if (ImGui::BeginMenu("File"))
    {
      if (ImGui::MenuItem("Save"))
        savePrj();
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit"))
    {
      if (ImGui::BeginMenu("Add"))
      {
        if (ImGui::MenuItem("Mouth..."))
          dialog = std::make_unique<FileOpen>("Add Mouth Dialog", [this](bool r, const auto &filePath) {
            if (r)
              addNode(Mouth::className, filePath.string());
          });
        if (ImGui::MenuItem("Eye..."))
          dialog = std::make_unique<FileOpen>("Add Eye Dialog", [this](bool r, const auto &filePath) {
            if (r)
              addNode(Eye::className, filePath.string());
          });

        if (ImGui::MenuItem("Sprite..."))
          dialog = std::make_unique<FileOpen>("Add Sprite Dialog", [this](bool r, const auto &filePath) {
            if (r)
              addNode(AnimSprite::className, filePath.string());
          });

        if (ImGui::MenuItem("Twitch Chat..."))
          dialog = std::make_unique<ChannelDialog>("mika314", [this](bool r, const auto &channel) {
            if (r)
              addNode(Chat::className, channel);
          });

        ImGui::EndMenu();
      }
      if (ImGui::MenuItem("Preferences..."))
        dialog = std::make_unique<PreferencesDialog>(preferences, [this](bool r) {
          if (r)
            lib.flush();
        });
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  if (dialog)
    if (!dialog->draw())
      dialog = nullptr;
  {
    ImGui::Begin("Outliner");
    ImGui::BeginDisabled(!selected);
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
    ImGui::EndDisabled();

    renderTree(*root);

    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    if (ImGui::Button("Hide UI"))
      showUi = false;
    ImGui::End();
  }
  {
    ImGui::Begin("Details");
    root->renderUi();
    if (selected)
      selected->renderUi();
    ImGui::End();
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
      if (editMode == EditMode::select)
      {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        const auto projMat = getProjMat();
        selected = root->nodeUnder(projMat, glm::vec2{1.f * mouseX, 1.f * mouseY});
      }
      else
        editMode = EditMode::select;
    }
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
      cancel();
  }
  if (!io.WantCaptureKeyboard || !io.WantCaptureMouse)
  {
    if (selected)
    {
      if (ImGui::IsKeyPressed(ImGuiKey_G))
      {
        editMode = EditMode::translate;
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        selected->translateStart(glm::vec2{1.f * mouseX, 1.f * mouseY});
      }

      if (ImGui::IsKeyPressed(ImGuiKey_S))
      {
        editMode = EditMode::scale;
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        selected->scaleStart(glm::vec2{1.f * mouseX, 1.f * mouseY});
      }

      if (ImGui::IsKeyPressed(ImGuiKey_R))
      {
        editMode = EditMode::rotate;
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        selected->rotStart(glm::vec2{1.f * mouseX, 1.f * mouseY});
      }
      if (ImGui::IsKeyPressed(ImGuiKey_X) || ImGui::IsKeyPressed(ImGuiKey_Delete))
      {
        Node::del(*selected);
        selected = nullptr;
      }
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
          auto n = saveFactory.ctor(className, name);
          n->loadAll(saveFactory, is);
          auto parent = selected->parent();
          selected = n.get();
          parent->addChild(std::move(n));
          editMode = EditMode::translate;
          int mouseX, mouseY;
          SDL_GetMouseState(&mouseX, &mouseY);
          selected->translateStart(glm::vec2{1.f * mouseX, 1.f * mouseY});
        }
      }
      if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        cancel();
    }
  }
}

auto App::cancel() -> void
{
  if (!selected)
  {
    editMode = EditMode::select;
    return;
  }
  switch (editMode)
  {
  case EditMode::select: selected = nullptr; break;
  case EditMode::translate: selected->translateCancel(); break;
  case EditMode::scale: selected->scaleCancel(); break;
  case EditMode::rotate: selected->rotCancel(); break;
  }
  editMode = EditMode::select;
}

auto App::tick(float dt) -> void
{
  audioCapture.tick();
  lib.tick(dt);

  if (!root)
    return;

  const auto projMat = getProjMat();
  int mouseX, mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);
  hovered = nullptr;
  const auto mousePos = glm::vec2{1.f * mouseX, 1.f * mouseY};
  switch (editMode)
  {
  case EditMode::select: hovered = root->nodeUnder(projMat, mousePos); break;
  case EditMode::translate: selected->translateUpdate(projMat, mousePos); break;
  case EditMode::scale: selected->scaleUpdate(projMat, mousePos); break;
  case EditMode::rotate: selected->rotUpdate(projMat, mousePos); break;
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
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen() && &v != root.get())
      selected = &v;
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
      selected = &v;
  }
}

static const auto ver = uint32_t{2};

auto App::loadPrj() -> void
{
  ImGui::LoadIniSettingsFromDisk("imgui.ini");

  std::ifstream st("prj.tpp", std::ofstream::binary);
  if (!st)
  {
    root = std::make_unique<Bouncer>(audioCapture);
    LOG("Error opening file prj.tpp");
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
    root = std::make_unique<Bouncer>(audioCapture);
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
    auto node = saveFactory.ctor(class_, name);
    auto oldSelected = selected;
    selected = node.get();
    if (!oldSelected)
      root->addChild(std::move(node));
    else
      oldSelected->addChild(std::move(node));
  }
  catch (const std::runtime_error &e)
  {
    postponedActions.emplace_back(
      [&]() { dialog = std::make_unique<MessageDialog>("Error", e.what()); });
    LOG(e.what());
  }
};
