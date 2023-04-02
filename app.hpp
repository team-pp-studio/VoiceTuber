#pragma once
#include "anim-sprite.hpp"
#include "audio-capture.hpp"
#include "bouncer.hpp"
#include "file-open.hpp"
#include "mouth.hpp"
#include "wav-2-visemes.hpp"
#include <functional>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <memory>

class App
{
public:
  App();
  auto render() -> void;
  auto renderUi() -> void;
  auto processIo() -> void;
  auto tick() -> void;

private:
  ImVec4 clearColor = ImVec4(123.f / 256.f, 164.f / 256.f, 119.f / 256.f, 1.00f);
  Wav2Visemes wav2Visemes;
  AudioCapture audioCapture;
  std::unique_ptr<Node> root;
  Node *hovered = nullptr;
  Node *selected = nullptr;
  enum class EditMode { select, move, scale, rotate };
  EditMode editMode = EditMode::select;
  glm::vec2 startMousePos;
  glm::vec2 initLoc;
  glm::vec2 initScale;
  float initRot;
  FileOpen addMouthDialog;
  FileOpen addAnimDialog;

  auto cancel() -> void;
  auto renderTree(Node &) -> void;
};
