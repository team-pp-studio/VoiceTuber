#pragma once

#include "wav.hpp"

class AudioSink
{
public:
  virtual ~AudioSink() = default;
  virtual auto ingest(Wav) -> void = 0;
};
