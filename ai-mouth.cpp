#include "ai-mouth.hpp"
#include "audio-in.hpp"
#include "audio-out.hpp"
#include "azure-stt.hpp"
#include "ui.hpp"
#include "undo.hpp"
#include <log/log.hpp>

AiMouth::AiMouth(Lib &aLib,
                 Undo &aUndo,
                 AudioIn &aAudioIn,
                 AudioOut &audioOut,
                 Wav2Visemes &aWav2Visemes,
                 const std::filesystem::path &path)
  : Sprite(aLib, aUndo, path),
    lib(aLib),
    audioIn(aAudioIn),
    wav2Visemes(aWav2Visemes),
    stt(lib.get().queryAzureStt()),
    tts(lib.get().queryAzureTts(audioOut)),
    twitch(aLib.queryTwitch("mika314"))
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

auto AiMouth::ingest(Wav wav, bool /*overlap*/) -> void
{
  wavBuf.insert(std::end(wavBuf), std::begin(wav), std::end(wav));
  using namespace std::chrono_literals;
  if (std::chrono::high_resolution_clock::now() > silStart + 1000ms)
  {
    const auto sampleRate = audioIn.get().sampleRate();
    if (static_cast<int>(wavBuf.size()) > 1 * sampleRate)
      stt->perform(Wav{std::begin(wavBuf), std::end(wavBuf)}, sampleRate, [this](std::string txt) {
        if (!txt.empty())
        {
          hostMsg += (hostMsg.empty() ? "" : "\n") + txt;
          LOG("host:", hostMsg.size(), txt);
        }
        if (hostMsg.size() < 75 || lib.get().gpt().queueSize() > 0)
          return;
        lib.get().gpt().prompt("Host", std::move(hostMsg), [this](std::string rsp) {
          LOG("co-host", rsp);
          tts->say("en-US-AmberNeural", std::move(rsp), false);
        });
        hostMsg.clear();
      });
    while (static_cast<int>(wavBuf.size()) > sampleRate / 5)
      wavBuf.pop_front();
  }
  if (std::chrono::high_resolution_clock::now() > silStart + 5000ms && hostMsg.size() > 5)
  {
    lib.get().gpt().prompt("Host", std::move(hostMsg), [this](std::string rsp) {
      LOG("co-host", rsp);
      tts->say("en-US-AmberNeural", std::move(rsp), false);
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
  Sprite::load(strm);
}

auto AiMouth::render(float dt, Node *hovered, Node *selected) -> void
{
  frame = viseme2Sprite[viseme] % numFrames;
  Sprite::render(dt, hovered, selected);
}

auto AiMouth::renderUi() -> void
{

  Sprite::renderUi();
  ImGui::TableNextColumn();

  {
    ImGui::PushStyleColor(
      ImGuiCol_Text,
      ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]); // Set text color to disabled color
    Ui::textRj("Viseme");
    ImGui::TableNextColumn();
    char str[16];
    switch (viseme)
    {
    case Viseme::sil: strcpy(str, "sil"); break;
    case Viseme::PP: strcpy(str, "PP"); break;
    case Viseme::FF: strcpy(str, "FF"); break;
    case Viseme::TH: strcpy(str, "TH"); break;
    case Viseme::DD: strcpy(str, "DD"); break;
    case Viseme::kk: strcpy(str, "kk"); break;
    case Viseme::CH: strcpy(str, "CH"); break;
    case Viseme::SS: strcpy(str, "SS"); break;
    case Viseme::nn: strcpy(str, "nn"); break;
    case Viseme::RR: strcpy(str, "RR"); break;
    case Viseme::aa: strcpy(str, "aa"); break;
    case Viseme::E: strcpy(str, "E"); break;
    case Viseme::I: strcpy(str, "I"); break;
    case Viseme::O: strcpy(str, "O"); break;
    case Viseme::U: strcpy(str, "U"); break;
    }
    ImGui::InputText("##Viseme", str, ImGuiInputTextFlags_ReadOnly);
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
        [&f, newF = f, this, vis]() {
          f = newF;
          viseme = vis;
          using namespace std::chrono_literals;
          freezeTime = std::chrono::high_resolution_clock::now() + 1s;
        },
        [&f, oldF, this, vis]() {
          f = oldF;
          viseme = vis;
          using namespace std::chrono_literals;
          freezeTime = std::chrono::high_resolution_clock::now() + 1s;
        });
    }
    ImGui::SameLine();

    char btnTitle[16];
    sprintf(btnTitle, "Test##%s", txt);

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
  Sprite::save(strm);
}

auto AiMouth::onMsg(Msg val) -> void
{
  lib.get().gpt().prompt(val.displayName, val.msg, [this](std::string rsp) {
    LOG("co-host", rsp);
    tts->say("en-US-AmberNeural", std::move(rsp), false);
  });
}
