#include "audio-out.hpp"

#include <algorithm>
#include <cstdint>

#include <spdlog/spdlog.h>

#include "preferences.hpp"

AudioOut::AudioOut(const std::string &device, int sampleRate, int frameSize)
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

auto AudioOut::updateDevice(const std::string &device) -> void
{
  audio = nullptr;
  audio = makeDevice(device);
}

auto AudioOut::ingest(Wav v, bool overlap) -> void
{
  audio->lock();
  if (overlap)
    for (auto i = 0U; i < v.size(); ++i)
    {
      while (i >= buf.size())
        buf.push_back(0);
      buf[i] += v[i];
    }
  else
    for (auto a : v)
      buf.push_back(a);
  audio->unlock();
}

auto AudioOut::sampleRate() const -> int
{
  return want.freq;
}

void AudioOut::callback(unsigned char *stream, int len)
{
  std::size_t const stream_len = len / sizeof(int16_t);
  auto const stream_begin = reinterpret_cast<int16_t *>(stream);
  auto const stream_end = stream_begin + stream_len;
  auto const to_copy = std::min(stream_len, buf.size());

  std::fill(std::copy_n(buf.cbegin(), to_copy, stream_begin), stream_end, 0);
  buf.erase(buf.begin(), buf.begin() + to_copy);
}

std::unique_ptr<sdl::Audio> AudioOut::makeDevice(const std::string &device)
{
  SDL_AudioSpec have;
  auto ret = std::make_unique<sdl::Audio>(
    device != Preferences::DefaultAudio ? device.c_str() : nullptr,
    0,
    &want,
    &have,
    0,
    std::bind_front(&AudioOut::callback, this));
  if (have.format != want.format)
    throw std::runtime_error("Failed to get the desired AudioSpec");
  ret->pause(0);
  return ret;
}
