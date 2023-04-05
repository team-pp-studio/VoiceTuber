#include "audio-capture.hpp"
#include <log/log.hpp>

AudioCapture::AudioCapture(int sampleRate, int frameSize)
  : want([sampleRate, frameSize]() {
      SDL_AudioSpec ret;
      SDL_zero(ret);
      ret.freq = sampleRate;
      ret.format = AUDIO_S16SYS;
      ret.channels = 1;
      ret.samples = frameSize;
      return ret;
    }()),
    audio(nullptr, 1, &want, &have, 0, [this](Uint8 *stream, int len) {
      buf_.insert(std::end(buf_),
                  reinterpret_cast<int16_t *>(stream),
                  reinterpret_cast<int16_t *>(stream) + len / sizeof(int16_t));
    })
{
  if (have.format != want.format)
    throw std::runtime_error("Failed to get the desired AudioSpec");

  audio.pause(0);
}

auto AudioCapture::tick() -> void
{
  audio.lock();
  auto v = std::move(buf_);
  buf_.clear();
  audio.unlock();

  if (sinks.empty())
    return;
  auto last = sinks.back();
  for (auto it = std::begin(sinks); it != std::end(sinks); ++it)
    if (&it->get() != &last.get())
      it->get().ingest(v);
    else
      it->get().ingest(std::move(v));
}

auto AudioCapture::reg(AudioSink &v) -> void
{
  sinks.push_back(v);
}

auto AudioCapture::unreg(AudioSink &v) -> void
{
  sinks.erase(
    std::remove_if(std::begin(sinks), std::end(sinks), [&](const auto &x) { return &x.get() == &v; }),
    std::end(sinks));
}
