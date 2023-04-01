#include "app.hpp"
#include <log/log.hpp>

static auto getProjMat() -> glm::mat4
{
  GLfloat projMatData[16];
  glGetFloatv(GL_PROJECTION_MATRIX, projMatData);
  return glm::make_mat4(projMatData);
}

App::App()
  : wav2Visemes([this](Viseme val) { mouth.setViseme(val); }),
    audioCapture(wav2Visemes.sampleRate(), wav2Visemes.frameSize()),
    face("face-blink-anim.png"),
    mouth("visemes.png")
{
  LOG("sample rate:", wav2Visemes.sampleRate());
  LOG("frame size:", wav2Visemes.frameSize());
  audioCapture.reg(wav2Visemes);
  audioCapture.reg(root);
  face.cols = 5;
  face.rows = 5;
  face.numFrames = 25;
  face.fps = 5.f;
  face.loc = {500.f, -50.f};
  face.pivot = {512.f, 512.f};
  mouth.cols = 4;
  mouth.rows = 4;
  mouth.loc = {382.f, 194.f};
  mouth.pivot = {128.f, 128.f};
  mouth.viseme2Sprite[Viseme::sil] = 6;
  mouth.viseme2Sprite[Viseme::PP] = 12;
  mouth.viseme2Sprite[Viseme::FF] = 9;
  mouth.viseme2Sprite[Viseme::TH] = 15;
  mouth.viseme2Sprite[Viseme::DD] = 8;
  mouth.viseme2Sprite[Viseme::kk] = 10;
  mouth.viseme2Sprite[Viseme::CH] = 7;
  mouth.viseme2Sprite[Viseme::SS] = 14;
  mouth.viseme2Sprite[Viseme::nn] = 11;
  mouth.viseme2Sprite[Viseme::RR] = 13;
  mouth.viseme2Sprite[Viseme::aa] = 1;
  mouth.viseme2Sprite[Viseme::E] = 4;
  mouth.viseme2Sprite[Viseme::I] = 2;
  mouth.viseme2Sprite[Viseme::O] = 5;
  mouth.viseme2Sprite[Viseme::U] = 3;
  mouth.numFrames = 16;
  root.addChild(face);
  face.addChild(mouth);
}

auto App::render() -> void
{
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
  glClear(GL_COLOR_BUFFER_BIT);

  root.renderAll(hovered, selected);
}

auto App::renderUi() -> void
{
  {
    ImGui::Begin("Z-order");

    ImGui::ColorEdit4("clear color", (float *)&clearColor); // Edit 3 floats representing a color

    ImGuiIO &io = ImGui::GetIO();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
  }

  ImGui::Begin("Details");
  root.renderUi();
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
        selected = root.nodeUnder(projMat, glm::vec2{1.f * mouseX, 1.f * mouseY});
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
  case EditMode::select: hovered = root.nodeUnder(projMat, endMousePos); break;
  case EditMode::move: selected->updateLoc(projMat, initLoc, startMousePos, endMousePos); break;
  case EditMode::scale: selected->updateScale(projMat, initScale, startMousePos, endMousePos); break;
  case EditMode::rotate: selected->updateRot(projMat, initRot, startMousePos, endMousePos); break;
  }
}
