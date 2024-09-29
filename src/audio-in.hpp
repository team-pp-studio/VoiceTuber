#pragma once
#include <memory>

#include <sdlpp/sdlpp.hpp>

#include "audio-sink.hpp"
#include "shared_from_this.hpp"
#include "uv.hpp"
#include "wav.hpp"

class AudioIn : public virtual enable_shared_from_this
{
public:
  AudioIn(uv::Uv &, const std::string &device, int sampleRate, int frameSize);
  AudioIn(AudioIn const &) = delete;
  AudioIn(AudioIn &&) = delete;

  AudioIn operator=(AudioIn const &) = delete;
  AudioIn operator=(AudioIn &&) = delete;

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

  void callback(unsigned char const *buf, int len);
  auto makeDevice(const std::string &device) -> std::unique_ptr<sdl::Audio>;
  auto tick() -> void;
};
