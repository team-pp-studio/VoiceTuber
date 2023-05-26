#pragma once
#include "audio-sink.hpp"

class AudioLevel final : public AudioSink
{
public:
  AudioLevel(class AudioIn &);
  AudioLevel(const AudioLevel &) = delete;
  ~AudioLevel() final;
  auto getLevel() const -> float;
  auto sampleRate() const -> int final;

private:
  std::reference_wrapper<AudioIn> audioIn;
  float level = 0.f;

  auto ingest(Wav) -> void final;
};
