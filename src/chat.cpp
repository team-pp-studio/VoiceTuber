#include "chat.hpp"
#include "audio-sink.hpp"
#include "imgui-helpers.hpp"
#include "lib.hpp"
#include "no-voice.hpp"
#include "ui.hpp"
#include "undo.hpp"
#include <scn/scn.h>
#include <sdlpp/sdlpp.hpp>
#include <spdlog/spdlog.h>

static const char *mute = "Mute";

Chat::Chat(class Lib &aLib, Undo &aUndo, uv::Uv &aUv, AudioSink &aAudioSink, std::string n)
  : Node(aLib, aUndo, n),
    lib(aLib),
    audioSink(aAudioSink),
    twitch(aLib.queryTwitch(n)),
    font(aLib.queryFont(sdl::get_base_path() / "assets/notepad_font/NotepadFont.ttf", ptsize)),
    timer(aUv.createTimer())
{
  twitch->reg(*this);
}

Chat::~Chat()
{
  twitch->unreg(*this);
}

static auto escName(std::string value) -> std::string
{
  std::transform(std::begin(value), std::end(value), std::begin(value), [](char ch) {
    if (ch == '_')
      return ' ';
    return ch;
  });
  while (!value.empty() && isdigit(value.back()))
    value.resize(value.size() - 1);
  return value;
}

static auto eq(const std::vector<std::string_view> &words, size_t i, size_t j, size_t w)
{
  if (i + w > words.size())
    return false;
  if (j + w > words.size())
    return false;
  for (auto k = 0U; k < w; ++k)
    if (words[i + k] != words[j + k])
      return false;
  return true;
}

static auto dedup(std::string_view var)
{
  std::vector<std::string_view> words;
  [[maybe_unused]] auto const result = scn::scan_list(var, words);
  assert(!result.error());
  for (bool didUpdate = true; didUpdate;)
  {
    didUpdate = false;
    for (auto w = 1U; w < words.size() / 2 && !didUpdate; ++w)
    {
      for (auto i = 0U; i < words.size() - w && !didUpdate; ++i)
      {
        for (auto r = 1U; !didUpdate; ++r)
        {
          if (!eq(words, i, i + r * w, w))
          {
            if (r >= 3)
            {
              words.erase(std::begin(words) + i + w, std::begin(words) + i + r * w);
              didUpdate = true;
            }
            else
              break;
          }
        }
      }
    }
  }

  return fmt::format("{}", fmt::join(words, " "));
}

static auto getDialogLine(const std::string &text, bool isMe)
{
  if (isMe)
    return "";
  if (text.find("?") != std::string::npos || text.find("!") == 0)
    return "asked:";
  if (text.find("!") != std::string::npos)
    return "yelled:";
  return "said:";
}

auto Chat::onMsg(Msg val) -> void
{
  showChat = true;
  timer.stop();
  if (hideChatSec > 0)
    timer.start(
      [alive = this->weak_from_this()]() {
        if (auto self = std::static_pointer_cast<Chat>(alive.lock()))
        {
          self->showChat = false;
        }
        else
        {
          SPDLOG_INFO("this was destroyed");
        }
      },
      hideChatSec * 1000);
  if (azureTts)
  {
    const auto displayName = val.displayName;
    const auto text = val.msg;
    const auto isMe = false; // val.isMe;
    const auto supressName = (lastName == displayName) && !isMe;
    const auto voice = getVoice(displayName);
    if (voice != mute)
      azureTts->say(
        voice,
        (!supressName ? (escName(displayName) + " " + getDialogLine(text, isMe) + " ") : "") +
          dedup(text));
    else
      audioSink.get().ingest(noVoice());
    lastName = displayName;
  }
  msgs.emplace_back(std::move(val));
}

static auto toLower(std::string v) -> std::string
{
  std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) { return std::tolower(c); });
  return v;
}

auto Chat::getVoice(const std::string &n) const -> std::string
{
  auto it = voicesMap.find(toLower(n));
  if (it != std::end(voicesMap))
    return it->second;
  if (voices.empty())
    return mute;
  return voices[(std::hash<std::string>()(n) ^ 1) % voices.size()];
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
  font = lib.get().queryFont(sdl::get_base_path() / "assets/notepad_font/NotepadFont.ttf", ptsize);
  if (tts)
  {
    if (!azureTts)
    {
      azureTts = lib.get().queryAzureTts(audioSink);
      azureTts->listVoices([alive = this->weak_from_this()](std::span<std::string_view> aVoices) {
        if (auto self = std::static_pointer_cast<Chat>(alive.lock()))
        {
          self->voices.insert(self->voices.begin(), aVoices.begin(), aVoices.end());
        }
        else
        {
          SPDLOG_INFO("this was destroyed");
        }
      });
    }
  }
}

auto Chat::render(float dt, Node *hovered, Node *selected) -> void
{
  if (!twitch->isConnected())
  {
    glColor4f(.5f, .5f, .5f, 1.f);
    glBegin(GL_LINES);
    glVertex2f(.0f, .0f);
    glVertex2f(w(), h());
    glVertex2f(w(), .0f);
    glVertex2f(.0f, h());
    glEnd();
  }
  if (!showChat)
  {
    Node::render(dt, hovered, selected);
    return;
  }
  auto y = 0.f;
  for (auto it = msgs.rbegin(); it != msgs.rend(); ++it)
  {
    const auto displayNameDim = font->getSize(it->displayName);
    auto const msg = fmt::format(": {}", it->msg);

    glColor3f(1.f, 1.f, 1.f);
    const auto wrappedLines = wrapText(msg, displayNameDim.x);
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

auto Chat::wrapText(std::string_view text, float initial_offset) const -> std::vector<std::string>
{
  std::vector<std::string> lines;
  std::string line;
  while (auto result = scn::scan_value<std::string_view>(text))
  {
    text = result.range_as_string_view();
    auto const word = result.value();

    if (line.empty())
      line += word;
    else
      (line += " ") += word;
    const auto text_dimensions = font->getSize(line);
    if (text_dimensions.x > w() - initial_offset)
    {
      initial_offset = 0;
      line.resize(line.size() == word.size() ? 0 : line.size() - word.size() - 1);
      lines.emplace_back(std::move(line));
      line = word;
    }
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
                "W##WidthEd",
                size.x,
                1.f,
                -std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                "%.1f");
  Ui::dragFloat(undo,
                "H##HeightEd",
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
      [newSize = ptsize, alive = this->weak_from_this()]() {
        if (auto self = std::static_pointer_cast<Chat>(alive.lock()))
        {
          self->ptsize = newSize;
          self->font = self->lib.get().queryFont(
            sdl::get_base_path() / "assets/notepad_font/NotepadFont.ttf", self->ptsize);
        }
        else
        {
          SPDLOG_INFO("this was destroyed");
        }
      },
      [oldSize, alive = this->weak_from_this()]() {
        if (auto self = std::static_pointer_cast<Chat>(alive.lock()))
        {
          self->ptsize = oldSize;
          self->font = self->lib.get().queryFont(
            sdl::get_base_path() / "assets/notepad_font/NotepadFont.ttf", self->ptsize);
        }
        else
        {
          SPDLOG_INFO("this was destroyed");
        }
      });
  ImGui::TableNextColumn();
  Ui::textRj("Hide Time");
  ImGui::TableNextColumn();
  Ui::inputInt(undo, "s##Hide Chat", hideChatSec);
  ImGui::TableNextColumn();
  Ui::textRj("Azure TTS");
  ImGui::TableNextColumn();
  {
    auto oldTts = tts;
    tts = static_cast<bool>(azureTts);
    if (ImGui::Checkbox("##AzureTTS", &tts))
    {
      undo.get().record(
        [alive = this->weak_from_this(), newTts = tts]() {
          if (auto self = std::static_pointer_cast<Chat>(alive.lock()))
          {
            if (newTts)
            {
              if (!self->azureTts)
              {
                self->azureTts = self->lib.get().queryAzureTts(self->audioSink);
                self->azureTts->listVoices([alive](std::span<std::string_view> aVoices) {
                  if (auto self = std::static_pointer_cast<Chat>(alive.lock()))
                  {
                    self->voices.clear();
                    self->voices.insert(self->voices.end(), aVoices.begin(), aVoices.end());
                  }
                  else
                  {
                    SPDLOG_INFO("this was destroyed");
                  }
                });
              }
            }
            else
            {
              self->azureTts = nullptr;
            }
          }
          else

          {
            SPDLOG_INFO("this was destroyed");
          }
        },
        [alive = this->weak_from_this(), oldTts]() {
          if (auto self = std::static_pointer_cast<Chat>(alive.lock()))
          {
            if (oldTts)
            {
              if (!self->azureTts)
              {
                self->azureTts = self->lib.get().queryAzureTts(self->audioSink);
                self->azureTts->listVoices([alive](std::span<std::string_view> voices) {
                  if (auto self = std::static_pointer_cast<Chat>(alive.lock()))
                  {
                    self->voices.insert(self->voices.end(), voices.begin(), voices.end());
                  }
                  else
                  {
                    SPDLOG_INFO("this was destroyed");
                  }
                });
              }
            }
            else
            {
              self->azureTts = nullptr;
            }
          }
          else

          {
            SPDLOG_INFO("this was destroyed");
          }
        });
    }
  }
  if (azureTts && !azureTts->lastError.empty())
  {
    ImGui::TableNextColumn();
    Ui::textRj("Error");
    ImGui::TableNextColumn();
    ImGui::TextF("{}", azureTts->lastError);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Voices Mapping");
  ImGui::TableNextColumn();

  {
    auto lines = std::vector<std::string>{};
    for (const auto &v : voicesMap)
      lines.emplace_back(v.first + " <-> " + v.second);
    std::sort(std::begin(lines), std::end(lines));
    for (const auto &line : lines)
      ImGui::TextUnformatted(line);
  }
  {
    ImGui::TableNextColumn();
    char chatterNameBuf[1024];
    strcpy(chatterNameBuf, chatterName.data());
    if (ImGui::InputText("##Chatter Name", chatterNameBuf, sizeof(chatterNameBuf)))
      chatterName = chatterNameBuf;
    ImGui::TableNextColumn();
    {
      auto combo = Ui::Combo("##Chatter Voice", chatterVoice.c_str(), 0);
      if (combo)
      {
        if (ImGui::Selectable((std::string{mute} + "##Voice").c_str(), chatterVoice == mute))
          chatterVoice = mute;
        for (const auto &voice : voices)
          if (ImGui::Selectable((voice + std::string{"##Voice"}).c_str(), chatterVoice == voice))
            chatterVoice = voice;
      }
    }
    ImGui::SameLine();
    // TODO-Mika undo/redo
    if (ImGui::Button("Add##AddVoiceMap"))
    {
      if (!chatterVoice.empty())
        voicesMap[toLower(chatterName)] = chatterVoice;
      else
        voicesMap.erase(chatterName);
    }
    ImGui::SameLine();
    if (ImGui::Button("Del##AddVoiceMap"))
      voicesMap.erase(toLower(chatterName));
  }

  if (!twitch->isConnected())
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 0.7f, 0.7f, 1.0f));

  ImGui::TableNextColumn();
  Ui::textRj("Chat");
  ImGui::TableNextColumn();
  if (auto chatListBox =
        Ui::ListBox{"##Chat", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())})
    for (const auto &msg : msgs)
      ImGui::TextF("{}: {}", msg.displayName, msg.msg);
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
