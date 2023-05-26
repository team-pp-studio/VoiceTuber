#include "audio-out.hpp"
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

auto AudioOut::ingest(Wav v) -> void
{
  audio->lock();
  for (auto i = 0U; i < v.size(); ++i)
  {
    while (i >= buf.size())
      buf.push_back(0);
    buf[i] += v[i];
  }
  audio->unlock();
}

auto AudioOut::sampleRate() const -> int
{
  return want.freq;
}

auto AudioOut::makeDevice(const std::string &device) -> std::unique_ptr<sdl::Audio>
{

  SDL_AudioSpec have;
  auto ret = std::make_unique<sdl::Audio>(device != Preferences::DefaultAudio ? device.c_str() : nullptr,
                                          0,
                                          &want,
                                          &have,
                                          0,
                                          [this](Uint8 *stream, int len) {
                                            for (auto i = 0U; i < len / sizeof(int16_t); ++i)
                                            {
                                              if (buf.empty())
                                                reinterpret_cast<int16_t *>(stream)[i] = 0;
                                              else
                                              {
                                                reinterpret_cast<int16_t *>(stream)[i] = buf.front();
                                                buf.pop_front();
                                              }
                                              // static int t = 0;
                                              // reinterpret_cast<int16_t *>(stream)[i] =
                                              //   0x1000 * sinf(2 * 3.14f * t++ * 440.f / 44100.f);
                                            }
                                          });
  if (have.format != want.format)
    throw std::runtime_error("Failed to get the desired AudioSpec");
  ret->pause(0);
  return ret;
}
