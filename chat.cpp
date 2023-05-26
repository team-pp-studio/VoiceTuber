#include "chat.hpp"
#include "lib.hpp"
#include "ui.hpp"
#include "undo.hpp"
#include <log/log.hpp>
#include <sstream>

Chat::Chat(class Lib &aLib,
           Undo &aUndo,
           class Uv &uv,
           HttpClient &aHttpClient,
           AudioSink &aAudioSink,
           std::string n)
  : Node(aLib, aUndo, n),
    lib(aLib),
    httpClient(aHttpClient),
    audioSink(aAudioSink),
    twitch(aLib.queryTwitch(uv, n)),
    font(aLib.queryFont(SDL_GetBasePath() + std::string{"assets/notepad_font/NotepadFont.ttf"}, ptsize)),
    timer(uv.getTimer())
{
  twitch->reg(*this);
}

Chat::~Chat()
{
  twitch->unreg(*this);
}

auto Chat::onMsg(Msg val) -> void
{
  showChat = true;
  timer.stop();
  timer.start([this]() { showChat = false; }, 30'000, false /*repeat*/);
  if (azureTts)
    azureTts->say("en-GB-MiaNeural", val.displayName + " said " + val.msg);
  msgs.emplace_back(std::move(val));
}

auto Chat::save(OStrm &strm) const -> void
{
  ::ser(strm, className);
  ::ser(strm, name);
  ::ser(strm, *this);
  Node::save(strm);
}
auto Chat::load(IStrm &strm) -> void
{
  ::deser(strm, *this);
  Node::load(strm);
  font =
    lib.get().queryFont(SDL_GetBasePath() + std::string{"assets/notepad_font/NotepadFont.ttf"}, ptsize);
  if (tts)
    azureTts = lib.get().queryAzureTts(httpClient, audioSink);
}

auto Chat::render(float dt, Node *hovered, Node *selected) -> void
{
  if (!showChat)
  {
    Node::render(dt, hovered, selected);
    return;
  }
  auto y = 0.f;
  for (auto it = msgs.rbegin(); it != msgs.rend(); ++it)
  {
    const auto displayNameDim = font->getSize(it->displayName);
    auto msg = std::ostringstream{};
    msg << ": " << it->msg;

    glColor3f(1.f, 1.f, 1.f);
    const auto wrappedLines = wrapText(msg.str(), displayNameDim.x);
    for (auto ln = wrappedLines.rbegin(); ln != wrappedLines.rend(); ++ln)
    {
      if (y > h())
        break;
      const auto isLast = ln == (wrappedLines.rend() - 1);
      font->render(glm::vec2{isLast ? displayNameDim.x : 0, y}, *ln);
      if (isLast)
      {
        glColor3f(it->color.x, it->color.y, it->color.z);
        font->render(glm::vec2{0.f, y}, it->displayName);
      }
      y += displayNameDim.y;
    }

    if (y > h())
      break;
  }

  Node::render(dt, hovered, selected);
}

auto Chat::wrapText(const std::string &text, float initOffset) const -> std::vector<std::string>
{
  std::vector<std::string> lines;
  auto iss = std::istringstream{text};
  std::string word;
  iss >> word;
  auto line = std::move(word);
  while (iss >> word)
  {
    auto tempLine = line + " " + word;
    const auto tempDim = font->getSize(tempLine);
    if (tempDim.x > w() - initOffset)
    {
      initOffset = 0;
      lines.emplace_back(std::move(line));
      line = word;
      continue;
    }
    line = std::move(tempLine);
  }
  if (!line.empty())
    lines.emplace_back(std::move(line));

  return lines;
}

auto Chat::renderUi() -> void
{
  Node::renderUi();
  ImGui::TableNextColumn();
  Ui::textRj("Size");
  ImGui::TableNextColumn();
  Ui::dragFloat(undo,
                "##width",
                size.x,
                1.f,
                -std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                "%.1f");
  Ui::dragFloat(undo,
                "##Height",
                size.y,
                1.f,
                -std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                "%.1f");

  ImGui::TableNextColumn();
  Ui::textRj("Font Size");
  ImGui::TableNextColumn();
  const auto oldSize = ptsize;
  if (ImGui::InputInt("##Font Size", &ptsize))
    undo.get().record(
      [newSize = ptsize, this]() {
        ptsize = newSize;
        font = lib.get().queryFont(
          SDL_GetBasePath() + std::string{"assets/notepad_font/NotepadFont.ttf"}, ptsize);
      },
      [oldSize, this]() {
        ptsize = oldSize;
        font = lib.get().queryFont(
          SDL_GetBasePath() + std::string{"assets/notepad_font/NotepadFont.ttf"}, ptsize);
      });
  ImGui::TableNextColumn();
  Ui::textRj("Azure TTS");
  ImGui::TableNextColumn();
  {
    auto oldTts = tts;
    tts = static_cast<bool>(azureTts);
    if (ImGui::Checkbox("##AzureTTS", &tts))
    {
      undo.get().record(
        [this, newTts = tts]() {
          if (newTts)
            azureTts = lib.get().queryAzureTts(httpClient, audioSink);
          else
            azureTts = nullptr;
        },
        [this, oldTts]() {
          if (oldTts)
            azureTts = lib.get().queryAzureTts(httpClient, audioSink);
          else
            azureTts = nullptr;
        });
    }
  }

  if (!twitch->isConnected())
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 0.7f, 0.7f, 1.0f));

  ImGui::TableNextColumn();
  Ui::textRj("Chat");
  ImGui::TableNextColumn();
  if (auto chatListBox =
        Ui::ListBox{"##Chat", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())})
    for (const auto &msg : msgs)
      ImGui::Text("%s: %s", msg.displayName.c_str(), msg.msg.c_str());
  if (!twitch->isConnected())
    ImGui::PopStyleColor();
}

auto Chat::h() const -> float
{
  return size.y;
}

auto Chat::w() const -> float
{
  return size.x;
}
