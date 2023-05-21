#pragma once
#include "audio-input.hpp"
#include "audio-output.hpp"
#include "dialog.hpp"
#include "lib.hpp"
#include "mouse-tracking.hpp"
#include "preferences.hpp"
#include "save-factory.hpp"
#include "twitch.hpp"
#include "undo.hpp"
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
  bool done = false;

private:
  enum class EditMode { select, translate, rotate, scale };

  Uv uv;
  Preferences preferences;
  SaveFactory saveFactory;
  Wav2Visemes wav2Visemes;
  AudioOutput audioOutput;
  AudioInput audioInput;
  MouseTracking mouseTracking;
  Lib lib;
  Undo undo;
  Node *hovered = nullptr;
  Node *selected = nullptr;
  bool isNodeDragging = false;
  std::unique_ptr<Dialog> dialog = nullptr;
  std::unique_ptr<Node> root;
  bool showUi = true;
  std::vector<std::function<auto()->void>> postponedActions;
  EditMode editMode = EditMode::select;
  std::shared_ptr<const Texture> selectIco;
  std::shared_ptr<const Texture> translateIco;
  std::shared_ptr<const Texture> scaleIco;
  std::shared_ptr<const Texture> rotateIco;
  std::shared_ptr<const Texture> selectDisabledIco;
  std::shared_ptr<const Texture> translateDisabledIco;
  std::shared_ptr<const Texture> scaleDisabledIco;
  std::shared_ptr<const Texture> rotateDisabledIco;
  std::shared_ptr<const Texture> hideUiIco;
  std::shared_ptr<const Texture> showUiIco;
  std::shared_ptr<const Texture> arrowN;
  std::shared_ptr<const Texture> arrowE;
  std::shared_ptr<const Texture> arrowS;
  std::shared_ptr<const Texture> arrowW;

  auto addNode(const std::string &class_, const std::string &name) -> void;
  auto cancel() -> void;
  auto loadPrj() -> void;
  auto renderTree(Node &) -> void;
};
