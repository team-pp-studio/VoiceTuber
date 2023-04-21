#pragma once
#include "audio-capture.hpp"
#include "dialog.hpp"
#include "lib.hpp"
#include "mouse-sink.hpp"
#include "save-factory.hpp"
#include "twitch.hpp"
#include "wav-2-visemes.hpp"
#include <functional>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <memory>

class App
{
public:
  App();
  auto render(float dt) -> void;
  auto renderUi(float dt) -> void;
  auto processIo() -> void;
  auto tick(float dt) -> void;
  auto savePrj() -> void;
  auto reg(MouseSink &) -> void;
  auto unreg(MouseSink &) -> void;

private:
  SaveFactory saveFactory;
  Wav2Visemes wav2Visemes;
  AudioCapture audioCapture;
  Lib lib;
  Node *hovered = nullptr;
  Node *selected = nullptr;
  enum class EditMode { select, translate, scale, rotate };
  EditMode editMode = EditMode::select;
  std::unique_ptr<Dialog> dialog = nullptr;
  std::vector<std::reference_wrapper<MouseSink>> mouseSinks;
  std::unique_ptr<Node> root;

  auto cancel() -> void;
  auto loadPrj() -> void;
  auto renderTree(Node &) -> void;
};
