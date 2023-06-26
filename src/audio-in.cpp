#include "audio-in.hpp"
#include "preferences.hpp"
#include <log/log.hpp>

AudioIn::AudioIn(uv::Uv &uv, const std::string &device, int sampleRate, int frameSize)
  : alive(std::make_shared<int>()),
    prepare(uv.createPrepare()),
    want([sampleRate, frameSize]() {
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
  prepare.start([alive = std::weak_ptr<int>(alive), this]() {
    if (!alive.lock())
    {
      LOG("this was destroyed");
      return;
    }
    tick();
  });
}

auto AudioIn::tick() -> void
{
  audio->lock();
  auto v = std::move(buf);
  buf.clear();
  audio->unlock();

  if (sinks.empty())
    return;
  auto last = sinks.back();
  for (auto it = std::begin(sinks); it != std::end(sinks); ++it)
    if (&it->get() != &last.get())
      it->get().ingest(v);
    else
      it->get().ingest(std::move(v));
}

auto AudioIn::reg(AudioSink &v) -> void
{
  sinks.push_back(v);
}

auto AudioIn::unreg(AudioSink &v) -> void
{
  sinks.erase(
    std::remove_if(std::begin(sinks), std::end(sinks), [&](const auto &x) { return &x.get() == &v; }),
    std::end(sinks));
}

auto AudioIn::updateDevice(const std::string &device) -> void
{
  audio = nullptr;
  audio = makeDevice(device);
}

auto AudioIn::makeDevice(const std::string &device) -> std::unique_ptr<sdl::Audio>
{
  SDL_AudioSpec have;
  auto ret = std::make_unique<sdl::Audio>(
    device != Preferences::DefaultAudio ? device.c_str() : nullptr,
    1,
    &want,
    &have,
    0,
    [alive = std::weak_ptr<int>(alive), this](Uint8 *stream, int len) {
      if (!alive.lock())
      {
        LOG("this was destroyed");
        return;
      }
      buf.insert(std::end(buf),
                 reinterpret_cast<int16_t *>(stream),
                 reinterpret_cast<int16_t *>(stream) + len / sizeof(int16_t));
    });
  if (have.format != want.format)
    throw std::runtime_error("Failed to get the desired AudioSpec");
  ret->pause(0);
  return ret;
}

auto AudioIn::sampleRate() const -> int
{
  return want.freq;
}
