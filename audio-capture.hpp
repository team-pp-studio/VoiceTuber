#pragma once
#include "audio-sink.hpp"
#include "wav.hpp"
#include <sdlpp/sdlpp.hpp>
#include <unordered_set>

class AudioCapture
{
public:
  AudioCapture(int sampleRate, int frameSize);
  auto reg(AudioSink &) -> void;
  auto unreg(AudioSink &) -> void;
  auto tick() -> void;

private:
  std::vector<std::reference_wrapper<AudioSink>> sinks;
  SDL_AudioSpec want;
  SDL_AudioSpec have;
  sdl::Audio audio;
  Wav buf_;
};
