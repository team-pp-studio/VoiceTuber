#include "app.hpp"
#include <log/log.hpp>

static auto getProjectionMatrix() -> glm::mat4
{
  GLfloat projectionMatrixData[16];
  glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrixData);
  return glm::make_mat4(projectionMatrixData);
}

App::App()
  : wav2Visemes([this](Viseme val) { curViseme = val; }),
    audioCapture(wav2Visemes.sampleRate(), wav2Visemes.frameSize()),
    sprite("visemes.png")
{
  LOG("sample rate:", wav2Visemes.sampleRate());
  LOG("frame size:", wav2Visemes.frameSize());
  audioCapture.reg(wav2Visemes);
  audioCapture.reg(root);
  sprite.cols = 3;
  sprite.rows = 5;
  sprite.loc = {.0f, 100.f};
  sprite.pivot = {512.f, 600.f - 293.f};
  root.nodes.push_back(sprite);
}

auto App::render() -> void
{
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
  glClear(GL_COLOR_BUFFER_BIT);

  root.renderAll();
}

auto App::renderUi() -> void
{
  {
    ImGui::Begin("Z-order");

    ImGui::ColorEdit4("clear color", (float *)&clearColor); // Edit 3 floats representing a color

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

    ImGuiIO &io = ImGui::GetIO();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
  }

  {
    ImGui::Begin("Details");
    root.renderUi();
    sprite.renderUi();
    ImGui::End();
  }
}

auto App::tick() -> void
{
  audioCapture.tick();
  sprite.viseme = curViseme;

  const auto projectionMatrix = getProjectionMatrix();
  int mouseX, mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);
  auto localPos = sprite.screenToLocal(projectionMatrix, Vec2{static_cast<float>(mouseX), static_cast<float>(mouseY)});
  LOG(localPos.x, localPos.y);
}
