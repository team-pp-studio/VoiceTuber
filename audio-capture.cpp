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

auto AudioCapture::buf() -> Wav
{
  audio.lock();
  Wav ret = std::move(buf_);
  buf_.clear();
  audio.unlock();
  return ret;
}
