#pragma once
#include "audio-sink.hpp"
#include "wav.hpp"
#include <memory>
#include <sdlpp/sdlpp.hpp>
#include <unordered_set>

class AudioInput
{
public:
  AudioInput(const std::string &device, int sampleRate, int frameSize);
  auto reg(AudioSink &) -> void;
  auto tick() -> void;
  auto unreg(AudioSink &) -> void;
  auto updateDevice(const std::string &device) -> void;

private:
  std::vector<std::reference_wrapper<AudioSink>> sinks;
  SDL_AudioSpec want;
  std::unique_ptr<sdl::Audio> audio;
  Wav buf_;

  auto makeDevice(const std::string &device) -> std::unique_ptr<sdl::Audio>;
};
