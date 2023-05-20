#include "audio-level.hpp"
#include "audio-input.hpp"

AudioLevel::AudioLevel(class AudioInput &aAudioInput) : audioInput(aAudioInput)
{
  audioInput.get().reg(*this);
}

AudioLevel::~AudioLevel()
{
  audioInput.get().unreg(*this);
}

auto AudioLevel::getLevel() const -> float
{
  return level;
}

auto AudioLevel::ingest(Wav wav) -> void
{
  for (auto v : wav)
  {
    if (v < 0)
      continue;
    auto curLevel = std::max(0.f, 0.14f * log(1.f * v / 0x7fff) + 1.f);
    level += 0.002f * (curLevel - level);
  }
}
