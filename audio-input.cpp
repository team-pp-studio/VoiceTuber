#include "audio-input.hpp"
#include "preferences.hpp"
#include <log/log.hpp>

AudioInput::AudioInput(const std::string &device, int sampleRate, int frameSize)
  : want([sampleRate, frameSize]() {
      SDL_AudioSpec ret;
      SDL_zero(ret);
      ret.freq = sampleRate;
      ret.format = AUDIO_S16SYS;
      ret.channels = 1;
      ret.samples = static_cast<Uint16>(frameSize);
      return ret;
    }()),
    audio(makeDevice(device))
{
}

auto AudioInput::tick() -> void
{
  audio->lock();
  auto v = std::move(buf_);
  buf_.clear();
  audio->unlock();

  if (sinks.empty())
    return;
  auto last = sinks.back();
  for (auto it = std::begin(sinks); it != std::end(sinks); ++it)
    if (&it->get() != &last.get())
      it->get().ingest(v);
    else
      it->get().ingest(std::move(v));
}

auto AudioInput::reg(AudioSink &v) -> void
{
  sinks.push_back(v);
}

auto AudioInput::unreg(AudioSink &v) -> void
{
  sinks.erase(
    std::remove_if(std::begin(sinks), std::end(sinks), [&](const auto &x) { return &x.get() == &v; }),
    std::end(sinks));
}

auto AudioInput::updateDevice(const std::string &device) -> void
{
  audio = nullptr;
  audio = makeDevice(device);
}

auto AudioInput::makeDevice(const std::string &device) -> std::unique_ptr<sdl::Audio>
{
  SDL_AudioSpec have;
  auto ret = std::make_unique<sdl::Audio>(device != Preferences::DefaultAudio ? device.c_str() : nullptr,
                                          1,
                                          &want,
                                          &have,
                                          0,
                                          [this](Uint8 *stream, int len) {
                                            buf_.insert(std::end(buf_),
                                                        reinterpret_cast<int16_t *>(stream),
                                                        reinterpret_cast<int16_t *>(stream) +
                                                          len / sizeof(int16_t));
                                          });
  if (have.format != want.format)
    throw std::runtime_error("Failed to get the desired AudioSpec");
  ret->pause(0);
  return ret;
}
