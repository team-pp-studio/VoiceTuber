#pragma once

#include "wav.hpp"

class AudioSink
{
public:
  virtual ~AudioSink() = default;
  virtual auto ingest(Wav, bool overlap = true) -> void = 0;
  virtual auto sampleRate() const -> int = 0;
};
