#pragma once
#include "wav.hpp"
#include <functional>
#include <pocketsphinx.h>

enum class Viseme { sil, PP, FF, TH, DD, kk, CH, SS, nn, RR, aa, E, I, O, U };

class Wav2Visemes
{
public:
  using Cb = std::function<auto(Viseme)->void>;
  Wav2Visemes(Cb);
  ~Wav2Visemes();
  auto ingest(Wav) -> void;
  auto sampleRate() const -> int;
  auto frameSize() const -> int;

private:
  Cb cb;
  ps_config_t *config = nullptr;
  ps_decoder_t *decoder = nullptr;
  ps_endpointer_t *ep = nullptr;
  Wav buf;
};
