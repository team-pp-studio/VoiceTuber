#define NOMINMAX

#include "audio-level.hpp"
#include "audio-in.hpp"

AudioLevel::AudioLevel(class AudioIn &aAudioIn) : audioIn(aAudioIn)
{
  audioIn.get().reg(*this);
}

AudioLevel::~AudioLevel()
{
  audioIn.get().unreg(*this);
}

auto AudioLevel::getLevel() const -> float
{
  return level;
}

auto AudioLevel::ingest(Wav wav, bool /*overlap*/) -> void
{
  for (auto v : wav)
  {
    if (v < 0)
      continue;
    auto curLevel = std::max(0.f, 0.14f * log(1.f * v / 0x7fff) + 1.f);
    level += 0.002f * (curLevel - level);
  }
}

auto AudioLevel::sampleRate() const -> int
{
  return audioIn.get().sampleRate();
}
