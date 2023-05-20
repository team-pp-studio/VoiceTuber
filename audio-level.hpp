#pragma once
#include "audio-sink.hpp"

class AudioLevel final : public AudioSink
{
public:
  AudioLevel(class AudioInput &);
  AudioLevel(const AudioLevel &) = delete;
  ~AudioLevel() final;
  auto getLevel() const -> float;

private:
  std::reference_wrapper<AudioInput> audioInput;
  float level = 0.f;

  auto ingest(Wav) -> void final;
};
