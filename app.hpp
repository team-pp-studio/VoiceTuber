#pragma once
#include "audio-capture.hpp"
#include "dialog.hpp"
#include "lib.hpp"
#include "mouse-tracking.hpp"
#include "preferences.hpp"
#include "save-factory.hpp"
#include "twitch.hpp"
#include "uv.hpp"
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

  bool isMinimized = false;

private:
  Uv uv;
  Preferences preferences;
  SaveFactory saveFactory;
  Wav2Visemes wav2Visemes;
  AudioCapture audioCapture;
  MouseTracking mouseTracking;
  Lib lib;
  Node *hovered = nullptr;
  Node *selected = nullptr;
  enum class EditMode { select, translate, scale, rotate };
  EditMode editMode = EditMode::select;
  std::unique_ptr<Dialog> dialog = nullptr;
  std::unique_ptr<Node> root;
  bool showUi = true;
  std::vector<std::function<auto()->void>> postponedActions;

  auto addNode(const std::string &class_, const std::string &name) -> void;
  auto cancel() -> void;
  auto loadPrj() -> void;
  auto renderTree(Node &) -> void;
};
