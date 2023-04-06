#include "app.hpp"
#include "anim-sprite.hpp"
#include "bouncer.hpp"
#include "mouth.hpp"
#include <fstream>
#include <log/log.hpp>

static auto getProjMat() -> glm::mat4
{
  GLfloat projMatData[16];
  glGetFloatv(GL_PROJECTION_MATRIX, projMatData);
  return glm::make_mat4(projMatData);
}

App::App()
  : audioCapture(wav2Visemes.sampleRate(), wav2Visemes.frameSize()),
    addMouthDialog("Add Mouth Dialog"),
    addSpriteDialog("Add Sprite Dialog"),
    lastUpdate(std::chrono::high_resolution_clock::now())
{
  LOG("sample rate:", wav2Visemes.sampleRate());
  LOG("frame size:", wav2Visemes.frameSize());
  audioCapture.reg(wav2Visemes);
  saveFactory.reg<Bouncer>([this](std::string) { return std::make_unique<Bouncer>(audioCapture); });
  saveFactory.reg<Mouth>(
    [this](std::string name) { return std::make_unique<Mouth>(wav2Visemes, texLib, std::move(name)); });
  saveFactory.reg<AnimSprite>(
    [this](std::string name) { return std::make_unique<AnimSprite>(texLib, std::move(name)); });
}

auto App::render() -> void
{

  if (!root)
  {
    glClearColor(.5f, .5f, .5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    return;
  }

  auto now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> diff = now - lastUpdate;
  lastUpdate = now;
  root->renderAll(diff.count(), hovered, selected);
}

auto App::renderUi() -> void
{
  if (!root)
  {
    if (prjDialog.draw())
      loadPrj();
    return;
  }
  {
    ImGui::Begin("Outliner");
    std::function<void(void)> postponedAction = nullptr;
    if (ImGui::Button("Save"))
      savePrj();
    ImGui::SameLine();
    if (ImGui::Button("Add Mouth..."))
      postponedAction = [&]() { ImGui::OpenPopup(addMouthDialog.dialogName); };
    ImGui::SameLine();
    if (ImGui::Button("Add Sprite..."))
      postponedAction = [&]() { ImGui::OpenPopup(addSpriteDialog.dialogName); };
    if (postponedAction)
      postponedAction();

    if (addMouthDialog.draw())
    {
      if (!std::filesystem::exists(addMouthDialog.getSelectedFile().filename()))
        std::filesystem::copy(addMouthDialog.getSelectedFile(),
                              addMouthDialog.getSelectedFile().filename());
      auto mouth = std::make_unique<Mouth>(
        wav2Visemes, texLib, addMouthDialog.getSelectedFile().filename().string());
      auto oldSelected = selected;
      selected = mouth.get();
      if (!oldSelected)
        root->addChild(std::move(mouth));
      else
        oldSelected->addChild(std::move(mouth));
    }
    if (addSpriteDialog.draw())
    {
      if (!std::filesystem::exists(addSpriteDialog.getSelectedFile().filename()))
        std::filesystem::copy(addSpriteDialog.getSelectedFile(),
                              addSpriteDialog.getSelectedFile().filename());
      auto sprite =
        std::make_unique<AnimSprite>(texLib, addSpriteDialog.getSelectedFile().filename().string());
      auto oldSelected = selected;
      selected = sprite.get();
      if (!oldSelected)
        root->addChild(std::move(sprite));
      else
        oldSelected->addChild(std::move(sprite));
    }

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

    ImGuiIO &io = ImGui::GetIO();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
  }
  {
    ImGui::Begin("Details");
    root->renderUi();
    if (selected)
      selected->renderUi();
    ImGui::End();
  }
}

auto App::processIo() -> void
{
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

auto App::tick() -> void
{
  audioCapture.tick();

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
