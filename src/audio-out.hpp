#pragma once
#include <deque>
#include <memory>
#include <string>

#include <sdlpp/sdlpp.hpp>

#include "audio-sink.hpp"
#include "shared_from_this.hpp"

class AudioOut final : public AudioSink, public virtual enable_shared_from_this
{
public:
  AudioOut(const std::string &device, int sampleRate = 44100, int frameSize = 1024);
  AudioOut(AudioOut const &) = delete;
  AudioOut(AudioOut &&) = delete;

  AudioOut operator=(AudioOut const &) = delete;
  AudioOut operator=(AudioOut &&) = delete;

  auto updateDevice(const std::string &) -> void;
  auto ingest(Wav, bool overlap) -> void final;
  auto sampleRate() const -> int final;

private:
  SDL_AudioSpec want;
  std::unique_ptr<sdl::Audio> audio;
  std::deque<int16_t> buf;

  void callback(unsigned char *, int);
  std::unique_ptr<sdl::Audio> makeDevice(const std::string &device);
};
