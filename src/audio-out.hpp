#pragma once
#include "audio-sink.hpp"
#include <deque>
#include <memory>
#include <sdlpp/sdlpp.hpp>
#include <string>

class AudioOut final : public AudioSink, public std::enable_shared_from_this<AudioOut>
{
public:
  AudioOut(const std::string &device, int sampleRate = 44100, int frameSize = 1024);
  auto updateDevice(const std::string &) -> void;
  auto ingest(Wav, bool overlap) -> void final;
  auto sampleRate() const -> int final;

private:
  SDL_AudioSpec want;
  std::unique_ptr<sdl::Audio> audio;
  std::deque<int16_t> buf;

  auto makeDevice(const std::string &device) -> std::unique_ptr<sdl::Audio>;
};
