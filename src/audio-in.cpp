#include "audio-in.hpp"

#include <spdlog/spdlog.h>

#include "preferences.hpp"

AudioIn::AudioIn(uv::Uv &uv, const std::string &device, int sampleRate, int frameSize)
  : prepare(uv.createPrepare()),
    want([sampleRate, frameSize]() {
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

  prepare.start(std::bind_front(&AudioIn::tick, this));
}

auto AudioIn::tick() -> void
{
  audio->lock();
  auto v = std::move(buf);
  buf.clear();
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

auto AudioIn::reg(AudioSink &v) -> void
{
  sinks.push_back(v);
}

auto AudioIn::unreg(AudioSink &v) -> void
{
  sinks.erase(
    std::remove_if(std::begin(sinks), std::end(sinks), [&](const auto &x) { return &x.get() == &v; }),
    std::end(sinks));
}

auto AudioIn::updateDevice(const std::string &device) -> void
{
  audio = nullptr;
  audio = makeDevice(device);
}

void AudioIn::callback(unsigned char const *stream, int len)
{
  std::size_t const stream_len = len / sizeof(int16_t);
  auto const stream_begin = reinterpret_cast<int16_t const *>(stream);
  auto const stream_end = stream_begin + stream_len;

  buf.insert(std::end(buf), stream_begin, stream_end);
}

std::unique_ptr<sdl::Audio> AudioIn::makeDevice(const std::string &device)
{
  SDL_AudioSpec have;
  auto ret = std::make_unique<sdl::Audio>(
    device != Preferences::DefaultAudio ? device.c_str() : nullptr,
    1,
    &want,
    &have,
    0,
    std::bind_front(&AudioIn::callback, this));
  if (have.format != want.format)
    throw std::runtime_error("Failed to get the desired AudioSpec");
  ret->pause(0);
  return ret;
}

auto AudioIn::sampleRate() const -> int
{
  return want.freq;
}
