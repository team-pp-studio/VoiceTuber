#include "audio-out.hpp"
#include "preferences.hpp"
#include <log/log.hpp>

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

auto AudioOut::makeDevice(const std::string &device) -> std::unique_ptr<sdl::Audio>
{
  SDL_AudioSpec have;
  auto ret = std::make_unique<sdl::Audio>(device != Preferences::DefaultAudio ? device.c_str() : nullptr,
                                          0,
                                          &want,
                                          &have,
                                          0,
                                          [alive = this->weak_from_this()](Uint8 *stream, int len) {
                                            if (auto self = alive.lock())
                                            {
                                              for (auto i = 0U; i < len / sizeof(int16_t); ++i)
                                              {
                                                if (self->buf.empty())
                                                  reinterpret_cast<int16_t *>(stream)[i] = 0;
                                                else
                                                {
                                                  reinterpret_cast<int16_t *>(stream)[i] =
                                                    self->buf.front();
                                                  self->buf.pop_front();
                                                }
                                                // static int t = 0;
                                                // reinterpret_cast<int16_t *>(stream)[i] =
                                                //   0x1000 * sinf(2 * 3.14f * t++ * 440.f / 44100.f);
                                              }
                                            }
                                            else

                                            {
                                              LOG("this was destroyed");
                                            }
                                          });
  if (have.format != want.format)
    throw std::runtime_error("Failed to get the desired AudioSpec");
  ret->pause(0);
  return ret;
}
