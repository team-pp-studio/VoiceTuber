#pragma once
#include "audio-sink.hpp"
#include "gpt.hpp"
#include "node.hpp"
#include "sprite-sheet.hpp"
#include "visemes-sink.hpp"

class AiMouth final : public AudioSink, public VisemesSink, public TwitchSink, public Node
{
public:
#define SER_PROP_LIST      \
  SER_PROP(viseme2Sprite); \
  SER_PROP(host);          \
  SER_PROP(cohost);        \
  SER_PROP(systemPrompt);  \
  SER_PROP(voice);

  SER_DEF_PROPS()
#undef SER_PROP_LIST

  AiMouth(Lib &,
          Undo &,
          class AudioIn &,
          class AudioOut &,
          class Wav2Visemes &,
          const std::filesystem::path &);
  ~AiMouth() final;

  static constexpr const char *className = "AiMouth";

private:
  SpriteSheet sprite;
  std::reference_wrapper<Lib> lib;
  std::reference_wrapper<AudioIn> audioIn;
  std::reference_wrapper<Wav2Visemes> wav2Visemes;
  std::shared_ptr<AzureStt> stt;
  std::shared_ptr<AzureTts> tts;
  std::shared_ptr<Twitch> twitch;
  Viseme viseme;
  std::chrono::high_resolution_clock::time_point freezeTime;
  std::map<Viseme, int> viseme2Sprite;
  std::string voice;
  std::deque<int16_t> wavBuf;
  std::chrono::high_resolution_clock::time_point silStart;
  std::string hostMsg;
  std::string systemPrompt;
  std::string host = "Mika";
  std::string cohost = "Clara";
  std::chrono::high_resolution_clock::time_point talkStart;

  auto h() const -> float final;
  auto ingest(Viseme) -> void final;
  auto ingest(Wav, bool overlap) -> void final;
  auto isTransparent(glm::vec2) const -> bool final;
  auto load(IStrm &) -> void final;
  auto onMsg(Msg) -> void final;
  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
  auto sampleRate() const -> int final;
  auto save(OStrm &) const -> void final;
  auto w() const -> float final;
};
