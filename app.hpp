#pragma once
#include "audio-capture.hpp"
#include "file-open.hpp"
#include "mouse-sink.hpp"
#include "prj-dialog.hpp"
#include "save-factory.hpp"
#include "tex-lib.hpp"
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
  auto savePrj() -> void;
  auto reg(MouseSink &) -> void;
  auto unreg(MouseSink &) -> void;

private:
  SaveFactory saveFactory;
  Wav2Visemes wav2Visemes;
  AudioCapture audioCapture;
  TexLib texLib;
  std::unique_ptr<Node> root;
  Node *hovered = nullptr;
  Node *selected = nullptr;
  enum class EditMode { select, translate, scale, rotate };
  EditMode editMode = EditMode::select;
  FileOpen addMouthDialog;
  FileOpen addEyeDialog;
  FileOpen addSpriteDialog;
  PrjDialog prjDialog;
  decltype(std::chrono::high_resolution_clock::now()) lastUpdate;
  std::vector<std::reference_wrapper<MouseSink>> mouseSinks;

  auto cancel() -> void;
  auto loadPrj() -> void;
  auto renderTree(Node &) -> void;
};
