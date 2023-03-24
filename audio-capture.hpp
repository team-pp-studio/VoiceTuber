#pragma once
#include "wav.hpp"
#include <sdlpp/sdlpp.hpp>

class AudioCapture
{
public:
  AudioCapture(int sampleRate, int frameSize);
  auto buf() -> Wav;

private:
  SDL_AudioSpec want;
  SDL_AudioSpec have;
  sdl::Audio audio;
  Wav buf_;
};
