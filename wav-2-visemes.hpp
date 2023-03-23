#pragma once
#include <functional>
#include <vector>

using Wav = std::vector<int16_t>;

enum class Viseme { sil, PP, FF, TH, DD, kk, CH, SS, nn, RR, aa, E, I, O, U };

class Wav2Visemes
{
public:
  using Cb = std::function<auto(Viseme)->void>;
  Wav2Visemes(Cb);
  auto ingest(Wav) -> void;
private:
  Cb cb;
};
