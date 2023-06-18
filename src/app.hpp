#pragma once
#include "audio-in.hpp"
#include "audio-out.hpp"
#include "azure-tts.hpp"
#include "dialog.hpp"
#include "http-client.hpp"
#include "lib.hpp"
#include "mouse-tracking.hpp"
#include "preferences.hpp"
#include "save-factory.hpp"
#include "twitch.hpp"
#include "undo.hpp"
#include "uv.hpp"
#include "wav-2-visemes.hpp"
#include <chrono>
#include <functional>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <memory>

class App
{
public:
  App(sdl::Window &, int argc, char *argv[]);
  ~App();
  auto tick() -> void;
  bool done = false;

private:
  enum class EditMode { select, translate, rotate, scale };

  std::reference_wrapper<sdl::Window> window;
  SDL_GLContext gl_context;
  decltype(std::chrono::high_resolution_clock::now()) lastUpdate;
  bool isMinimized = false;
  uv::Uv uv;
  Preferences preferences;
  SaveFactory saveFactory;
  Wav2Visemes wav2Visemes;
  AudioOut audioOut;
  AudioIn audioIn;
  MouseTracking mouseTracking;
  HttpClient httpClient;
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
  int originalX, originalY;
  int width, height;
  uv::Timer renderTimer;
  uv::Idle renderIdle;

  auto addNode(const std::string &class_, const std::string &name) -> void;
  auto cancel() -> void;
  auto droppedFile(std::string) -> void;
  auto loadPrj() -> void;
  auto processIo() -> void;
  auto render(float dt) -> void;
  auto renderTree(Node &) -> void;
  auto renderUi(float dt) -> void;
  auto savePrj() -> void;
  auto sdlEventsAndRender() -> void;
  auto setupRendering() -> void;
};
