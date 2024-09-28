#include "ai-mouth.hpp"
#include "audio-in.hpp"
#include "audio-out.hpp"
#include "azure-stt.hpp"
#include "ui.hpp"
#include "undo.hpp"
#include "wav-2-visemes.hpp"
#include <fmt/core.h>
#include <spdlog/spdlog.h>

AiMouth::AiMouth(Lib &aLib,
                 Undo &aUndo,
                 AudioIn &aAudioIn,
                 AudioOut &audioOut,
                 Wav2Visemes &aWav2Visemes,
                 const std::filesystem::path &path)
  : Node(aLib, aUndo, path.filename().string()),
    sprite(aLib, aUndo, path),
    lib(aLib),
    audioIn(aAudioIn),
    wav2Visemes(aWav2Visemes),
    stt(lib.get().queryAzureStt()),
    tts(lib.get().queryAzureTts(audioOut)),
    twitch(aLib.queryTwitch("mika314")),
    systemPrompt(lib.get().gpt().systemPrompt())
{
  viseme2Sprite[Viseme::sil] = 0;
  viseme2Sprite[Viseme::PP] = 1;
  viseme2Sprite[Viseme::FF] = 2;
  viseme2Sprite[Viseme::TH] = 3;
  viseme2Sprite[Viseme::DD] = 4;
  viseme2Sprite[Viseme::kk] = 5;
  viseme2Sprite[Viseme::CH] = 6;
  viseme2Sprite[Viseme::SS] = 7;
  viseme2Sprite[Viseme::nn] = 8;
  viseme2Sprite[Viseme::RR] = 9;
  viseme2Sprite[Viseme::aa] = 10;
  viseme2Sprite[Viseme::E] = 11;
  viseme2Sprite[Viseme::I] = 12;
  viseme2Sprite[Viseme::O] = 13;
  viseme2Sprite[Viseme::U] = 14;
  wav2Visemes.get().reg(*this);
  audioIn.get().reg(*this);
  twitch->reg(*this);
}

AiMouth::~AiMouth()
{
  wav2Visemes.get().unreg(*this);
  audioIn.get().unreg(*this);
  twitch->unreg(*this);
}

auto AiMouth::do_clone() const -> std::shared_ptr<Node>
{
  return std::make_shared<AiMouth>(*this);
}

auto AiMouth::ingest(Wav wav, bool /*overlap*/) -> void
{
  if (!visible)
    return;
  wavBuf.insert(std::end(wavBuf), std::begin(wav), std::end(wav));
  using namespace std::chrono_literals;
  if (std::chrono::high_resolution_clock::now() > silStart + 1000ms)
  {
    const auto sampleRate = audioIn.get().sampleRate();
    if (static_cast<int>(wavBuf.size()) > 1 * sampleRate)
    {
      auto max = std::max_element(std::begin(wavBuf), std::end(wavBuf));
      if (*max > 0x2000 || static_cast<int>(wavBuf.size()) > 10 * sampleRate)
        stt->perform(Wav{std::begin(wavBuf), std::end(wavBuf)},
                     sampleRate,
                     [alive = weak_from_this()](std::string_view txt) {
                       if (auto self = std::static_pointer_cast<AiMouth>(alive.lock()))
                       {
                         if (!self->hostMsg.empty())
                           self->hostMsg += '\n';
                         self->hostMsg += txt;
                         SPDLOG_INFO("{}: {}", self->host, txt);
                         if (self->hostMsg.size() < 75)
                           return;

                         self->lib.get().gpt().prompt(
                           "Host", std::move(self->hostMsg), [alive](std::string_view rsp) {
                             if (auto self = std::static_pointer_cast<AiMouth>(alive.lock()))
                             {
                               SPDLOG_INFO("{}: {}", self->cohost, rsp);
                               self->tts->say("en-US-AmberNeural", std::string(rsp), false);
                               self->talkStart = std::chrono::high_resolution_clock::now();
                             }
                             else
                             {
                               SPDLOG_INFO("this was destroyed");
                             };
                           });
                         self->hostMsg.clear();
                       }
                       else
                       {
                         SPDLOG_INFO("this was destroyed");
                       }
                     });
    }
    while (static_cast<int>(wavBuf.size()) > sampleRate / 5)
      wavBuf.pop_front();
  }
  if (std::chrono::high_resolution_clock::now() > silStart + 5000ms && hostMsg.size() > 5)
  {
    lib.get().gpt().prompt(host, std::move(hostMsg), [alive = weak_from_this()](std::string_view rsp) {
      if (auto self = std::static_pointer_cast<AiMouth>(alive.lock()))
      {
        if (rsp.empty())
          return;
        SPDLOG_INFO("{}: {}", self->cohost, rsp);
        self->tts->say("en-US-AmberNeural", std::string(rsp), false);
        self->talkStart = std::chrono::high_resolution_clock::now();
      }
      else
      {
        SPDLOG_INFO("this was destroyed");
      }
    });
    hostMsg.clear();
  }
}

auto AiMouth::ingest(Viseme v) -> void
{
  viseme = v;
  if (v != Viseme::sil)
    silStart = std::chrono::high_resolution_clock::now();
}

auto AiMouth::sampleRate() const -> int
{
  return audioIn.get().sampleRate();
}

auto AiMouth::load(IStrm &strm) -> void
{
  ::deser(strm, *this);
  sprite.load(strm);
  Node::load(strm);
  lib.get().gpt().systemPrompt(systemPrompt);
  lib.get().gpt().cohost(cohost);
}

auto AiMouth::render(float dt, Node *hovered, Node *selected) -> void
{
  using namespace std::chrono_literals;
  if (std::chrono::high_resolution_clock::now() < talkStart + 3s && sprite.numFrames() > 0)
    sprite.frame(rand() % sprite.numFrames());
  else
    sprite.frame(0);
  sprite.render();
  Node::render(dt, hovered, selected);
}

auto AiMouth::renderUi() -> void
{
  Node::renderUi();
  sprite.renderUi();
  ImGui::TableNextColumn();
  Ui::textRj("System Prompt");
  ImGui::TableNextColumn();
  {
    char text[1024 * 16];
    std::copy(std::begin(systemPrompt), std::end(systemPrompt), text);
    text[systemPrompt.size()] = '\0';
    if (ImGui::InputTextMultiline("##system prompt",
                                  text,
                                  IM_ARRAYSIZE(text),
                                  ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16),
                                  0))
    {
      systemPrompt = text;
      lib.get().gpt().systemPrompt(systemPrompt);
    }
  }
  ImGui::TableNextColumn();
  Ui::textRj("Host");
  ImGui::TableNextColumn();
  {
    char text[1024 * 16];
    std::copy(std::begin(host), std::end(host), text);
    text[host.size()] = '\0';
    if (ImGui::InputText("##host", text, IM_ARRAYSIZE(text)))
      host = text;
  }
  ImGui::TableNextColumn();
  Ui::textRj("Cohost");
  ImGui::TableNextColumn();
  {
    char text[1024 * 16];
    std::copy(std::begin(cohost), std::end(cohost), text);
    text[cohost.size()] = '\0';
    if (ImGui::InputText("##cohost", text, IM_ARRAYSIZE(text)))
    {
      cohost = text;
      lib.get().gpt().cohost(cohost);
    }
  }
  ImGui::TableNextColumn();
  {
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]); // Set text color to disabled color
    Ui::textRj("Viseme");
    ImGui::TableNextColumn();
    std::string_view str;
    switch (viseme)
    {
      using namespace std::literals;
    case Viseme::sil: str = "sil"sv; break;
    case Viseme::PP: str = "PP"sv; break;
    case Viseme::FF: str = "FF"sv; break;
    case Viseme::TH: str = "TH"sv; break;
    case Viseme::DD: str = "DD"sv; break;
    case Viseme::kk: str = "kk"sv; break;
    case Viseme::CH: str = "CH"sv; break;
    case Viseme::SS: str = "SS"sv; break;
    case Viseme::nn: str = "nn"sv; break;
    case Viseme::RR: str = "RR"sv; break;
    case Viseme::aa: str = "aa"sv; break;
    case Viseme::E: str = "E"sv; break;
    case Viseme::I: str = "I"sv; break;
    case Viseme::O: str = "O"sv; break;
    case Viseme::U: str = "U"sv; break;
    }
    ImGui::InputText("##Viseme", const_cast<char *>(str.data()), str.size(), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor(); // Restore the original text color
  }

  auto visUi = [&](auto vis, auto txt, auto txt2) {
    auto &f = viseme2Sprite[vis];
    ImGui::TableNextColumn();
    Ui::textRj(txt);
    ImGui::TableNextColumn();
    const auto oldF = f;
    if (ImGui::InputInt(txt2, &f))
    {
      undo.get().record(
        [&f, newF = f, alive = weak_from_this(), vis]() {
          if (auto self = std::static_pointer_cast<AiMouth>(alive.lock()))
          {
            using namespace std::chrono_literals;
            f = std::move(newF);
            self->viseme = std::move(vis);
            self->freezeTime = std::chrono::high_resolution_clock::now() + 1s;
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        },
        [&f, oldF, alive = weak_from_this(), vis]() {
          if (auto self = std::static_pointer_cast<AiMouth>(alive.lock()))
          {
            using namespace std::chrono_literals;
            f = std::move(oldF);
            self->viseme = std::move(vis);
            self->freezeTime = std::chrono::high_resolution_clock::now() + 1s;
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        });
    }
    ImGui::SameLine();

    char btnTitle[16];
    *fmt::format_to_n(btnTitle, std::size(btnTitle) - 1, "Test##{}", txt).out = 0;

    if (ImGui::Button(btnTitle))
    {
      viseme = vis;
      using namespace std::chrono_literals;
      freezeTime = std::chrono::high_resolution_clock::now() + 1s;
    }
  };
  visUi(Viseme::sil, "sil", "##sil");
  visUi(Viseme::PP, "PP", "##PP");
  visUi(Viseme::FF, "FF", "##FF");
  visUi(Viseme::TH, "TH", "##TH");
  visUi(Viseme::DD, "DD", "##DD");
  visUi(Viseme::kk, "kk", "##kk");
  visUi(Viseme::CH, "CH", "##CH");
  visUi(Viseme::SS, "SS", "##SS");
  visUi(Viseme::nn, "nn", "##nn");
  visUi(Viseme::RR, "RR", "##RR");
  visUi(Viseme::aa, "aa", "##aa");
  visUi(Viseme::E, "E", "##E");
  visUi(Viseme::I, "I", "##I");
  visUi(Viseme::O, "O", "##O");
  visUi(Viseme::U, "U", "##U");
}

auto AiMouth::save(OStrm &strm) const -> void
{
  ::ser(strm, className);
  ::ser(strm, name);
  ::ser(strm, *this);
  sprite.save(strm);
  Node::save(strm);
}

auto AiMouth::onMsg(Msg val) -> void
{
  lib.get().gpt().prompt(
    val.displayName + " from chat", val.msg, [alive = weak_from_this()](std::string_view rsp) {
      if (auto self = std::static_pointer_cast<AiMouth>(alive.lock()))
      {
        if (rsp.empty())
          return;

        SPDLOG_INFO("{}: {}", self->cohost, rsp);
        self->tts->say("en-US-AmberNeural", std::string(rsp), false);
        self->talkStart = std::chrono::high_resolution_clock::now();
      }
      else
      {
        SPDLOG_INFO("this was destroyed");
      }
    });
}

auto AiMouth::h() const -> float
{
  return sprite.h();
}

auto AiMouth::isTransparent(glm::vec2 v) const -> bool
{
  return sprite.isTransparent(v);
}

auto AiMouth::w() const -> float
{
  return sprite.w();
}
