#pragma once
#include "audio-sink.hpp"
#include "uv.hpp"
#include "wav.hpp"
#include <memory>
#include <sdlpp/sdlpp.hpp>
#include <unordered_set>

class AudioIn : public std::enable_shared_from_this<AudioIn>
{
public:
  AudioIn(uv::Uv &, const std::string &device, int sampleRate, int frameSize);
  auto reg(AudioSink &) -> void;
  auto unreg(AudioSink &) -> void;
  auto updateDevice(const std::string &device) -> void;
  auto sampleRate() const -> int;

private:
  uv::Prepare prepare;
  std::vector<std::reference_wrapper<AudioSink>> sinks;
  SDL_AudioSpec want;
  std::unique_ptr<sdl::Audio> audio;
  Wav buf;

  auto makeDevice(const std::string &device) -> std::unique_ptr<sdl::Audio>;
  auto tick() -> void;
};
