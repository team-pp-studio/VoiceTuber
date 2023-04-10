#pragma once
#include "audio-sink.hpp"
#include "viseme.hpp"
#include "visemes-sink.hpp"
#include "wav.hpp"
#include <functional>
#include <pocketsphinx.h>

class Wav2Visemes final : public AudioSink
{
public:
  Wav2Visemes();
  ~Wav2Visemes() final;
  auto ingest(Wav) -> void final;
  auto sampleRate() const -> int;
  auto frameSize() const -> int;
  auto reg(VisemesSink &) -> void;
  auto unreg(VisemesSink &) -> void;

private:
  std::vector<std::reference_wrapper<VisemesSink>> sinks;
  ps_config_t *config = nullptr;
  ps_decoder_t *decoder = nullptr;
  ps_endpointer_t *ep = nullptr;
  Wav buf;
};
