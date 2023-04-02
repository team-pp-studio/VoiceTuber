#include "app.hpp"
#include <log/log.hpp>

static auto getProjMat() -> glm::mat4
{
  GLfloat projMatData[16];
  glGetFloatv(GL_PROJECTION_MATRIX, projMatData);
  return glm::make_mat4(projMatData);
}

App::App()
  : wav2Visemes([this](Viseme val) {
      mouth1.setViseme(val);
      mouth2.setViseme(val);
    }),
    audioCapture(wav2Visemes.sampleRate(), wav2Visemes.frameSize()),
    face("face-blink-anim.png"),
    mouth1("visemes.png"),
    mouth2("visemes.png"),
    root(bouncer)
{
  LOG("sample rate:", wav2Visemes.sampleRate());
  LOG("frame size:", wav2Visemes.frameSize());
  audioCapture.reg(wav2Visemes);
  audioCapture.reg(bouncer);
  face.cols = 5;
  face.rows = 5;
  face.numFrames = 25;
  face.fps = 5.f;
  face.loc = {500.f, -50.f};
  face.pivot = {512.f, 512.f};
  mouth1.cols = 4;
  mouth1.rows = 4;
  mouth1.loc = {382.f, 194.f};
  mouth1.pivot = {128.f, 128.f};
  mouth1.viseme2Sprite[Viseme::sil] = 6;
  mouth1.viseme2Sprite[Viseme::PP] = 12;
  mouth1.viseme2Sprite[Viseme::FF] = 9;
  mouth1.viseme2Sprite[Viseme::TH] = 15;
  mouth1.viseme2Sprite[Viseme::DD] = 8;
  mouth1.viseme2Sprite[Viseme::kk] = 10;
  mouth1.viseme2Sprite[Viseme::CH] = 7;
  mouth1.viseme2Sprite[Viseme::SS] = 14;
  mouth1.viseme2Sprite[Viseme::nn] = 11;
  mouth1.viseme2Sprite[Viseme::RR] = 13;
  mouth1.viseme2Sprite[Viseme::aa] = 1;
  mouth1.viseme2Sprite[Viseme::E] = 4;
  mouth1.viseme2Sprite[Viseme::I] = 2;
  mouth1.viseme2Sprite[Viseme::O] = 5;
  mouth1.viseme2Sprite[Viseme::U] = 3;
  mouth1.numFrames = 16;

  mouth2.cols = 4;
  mouth2.rows = 4;
  mouth2.loc = {382.f, 194.f};
  mouth2.pivot = {128.f, 128.f};
  mouth2.viseme2Sprite[Viseme::sil] = 6;
  mouth2.viseme2Sprite[Viseme::PP] = 12;
  mouth2.viseme2Sprite[Viseme::FF] = 9;
  mouth2.viseme2Sprite[Viseme::TH] = 15;
  mouth2.viseme2Sprite[Viseme::DD] = 8;
  mouth2.viseme2Sprite[Viseme::kk] = 10;
  mouth2.viseme2Sprite[Viseme::CH] = 7;
  mouth2.viseme2Sprite[Viseme::SS] = 14;
  mouth2.viseme2Sprite[Viseme::nn] = 11;
  mouth2.viseme2Sprite[Viseme::RR] = 13;
  mouth2.viseme2Sprite[Viseme::aa] = 1;
  mouth2.viseme2Sprite[Viseme::E] = 4;
  mouth2.viseme2Sprite[Viseme::I] = 2;
  mouth2.viseme2Sprite[Viseme::O] = 5;
  mouth2.viseme2Sprite[Viseme::U] = 3;
  mouth2.numFrames = 16;
  bouncer.addChild(face);
  face.addChild(mouth1);
  face.addChild(mouth2);
}

auto App::render() -> void
{
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
  glClear(GL_COLOR_BUFFER_BIT);

  bouncer.renderAll(hovered, selected);
}

auto App::renderUi() -> void
{
  {
    ImGui::Begin("Z-order");
    ImGui::ColorEdit4("clear color", (float *)&clearColor); // Edit 3 floats representing a color

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

    renderTree(root);

    ImGuiIO &io = ImGui::GetIO();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
  }

  ImGui::Begin("Details");
  root.get().renderUi();
  if (selected)
    selected->renderUi();
  ImGui::End();
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
        selected = root.get().nodeUnder(projMat, glm::vec2{1.f * mouseX, 1.f * mouseY});
      }
      else
        editMode = EditMode::select;
    }
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
      cancel();
  }
  if (!io.WantCaptureKeyboard)
  {
    if (selected)
    {
      // Check if 'G' key was pressed for move operation
      if (ImGui::IsKeyPressed(ImGuiKey_G))
      {
        LOG("'G' key pressed, handle move operation here");
        editMode = EditMode::move;
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        startMousePos = glm::vec2{1.f * mouseX, 1.f * mouseY};
        initLoc = selected->loc;
      }

      // Check if 'S' key was pressed for scale operation
      if (ImGui::IsKeyPressed(ImGuiKey_S))
      {
        LOG("'S' key pressed, handle scale operation here");
        editMode = EditMode::scale;
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        startMousePos = glm::vec2{1.f * mouseX, 1.f * mouseY};
        initScale = selected->scale;
      }

      // Check if 'R' key was pressed for rotate operation
      if (ImGui::IsKeyPressed(ImGuiKey_R))
      {
        LOG("'R' key pressed, handle rotate operation here");
        editMode = EditMode::rotate;
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        startMousePos = glm::vec2{1.f * mouseX, 1.f * mouseY};
        initRot = selected->rot;
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
  case EditMode::move: selected->loc = initLoc; break;
  case EditMode::scale: selected->scale = initScale; break;
  case EditMode::rotate: selected->rot = initRot; break;
  }
  editMode = EditMode::select;
}

auto App::tick() -> void
{
  audioCapture.tick();

  const auto projMat = getProjMat();
  int mouseX, mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);
  hovered = nullptr;
  const auto endMousePos = glm::vec2{1.f * mouseX, 1.f * mouseY};
  switch (editMode)
  {
  case EditMode::select: hovered = root.get().nodeUnder(projMat, endMousePos); break;
  case EditMode::move: selected->updateLoc(projMat, initLoc, startMousePos, endMousePos); break;
  case EditMode::scale: selected->updateScale(projMat, initScale, startMousePos, endMousePos); break;
  case EditMode::rotate: selected->updateRot(projMat, initRot, startMousePos, endMousePos); break;
  }
}

auto App::renderTree(Node &v) -> void
{
  ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                 ImGuiTreeNodeFlags_SpanAvailWidth;
  ImGuiTreeNodeFlags nodeFlags = baseFlags;

  const auto nm = v.name();
  if (selected == &v)
    nodeFlags |= ImGuiTreeNodeFlags_Selected;
  const auto &nodes = v.nodes();
  if (!nodes.empty())
  {
    const auto nodeOpen = ImGui::TreeNodeEx(&v, nodeFlags, "%s", nm.c_str());
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen() && &v != &root.get())
      selected = &v;
    if (nodeOpen)
    {
      for (const auto &n : nodes)
        renderTree(n);
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
