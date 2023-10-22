#include "wav-2-visemes.hpp"
#include <pocketsphinx.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

Wav2Visemes::Wav2Visemes()
  : config([]() {
      auto ret = ps_config_init(nullptr);
      ps_default_search_args(ret);
      ps_config_set_str(ret, "lm", nullptr);
      ps_config_set_str(ret, "allphone", "assets/pocketsphinx-model/en-us/en-us-phone.lm.bin");
      ps_config_set_str(ret, "hmm", "assets/pocketsphinx-model/en-us/en-us");
      ps_config_set_bool(ret, "backtrace", TRUE);
      ps_config_set_float(ret, "beam", 1e-20);
      ps_config_set_float(ret, "lw", 2.0);

      return ret;
    }()),
    decoder([this]() {
      auto ret = ps_init(config);
      if (!ret)
        throw std::runtime_error("PocketSphinx decoder init failed");
      return ret;
    }()),
    ep([]() {
      auto ret = ps_endpointer_init(0.15f, 0.45f, PS_VAD_LOOSE, 0, 0);
      if (!ret)
        throw std::runtime_error("PocketSphinx endpointer init failed");
      return ret;
    }())
{
}

Wav2Visemes::~Wav2Visemes()
{
  ps_endpointer_free(ep);
  ps_free(decoder);
  ps_config_free(config);
}
auto Wav2Visemes::ingest(Wav wav, bool /*overlap*/) -> void
{
  while (!wav.empty())
  {
    const auto fs = frameSize();
    const auto c = std::min(static_cast<int>(wav.size()), fs);
    buf.insert(std::end(buf), std::begin(wav), std::begin(wav) + c);
    wav.erase(std::begin(wav), std::begin(wav) + c);
    if (static_cast<int>(buf.size()) != fs)
      continue;
    const auto prevInSpeech = ps_endpointer_in_speech(ep);

    auto speech = ps_endpointer_process(ep, buf.data());
    buf.clear();
    if (!speech)
      continue;
    if (!prevInSpeech)
      ps_start_utt(decoder);
    const auto ret = ps_process_raw(decoder, speech, fs, FALSE, FALSE);
    if (ret < 0)
      throw std::runtime_error("ps_process_raw() failed");
    const auto hyp = ps_get_hyp(decoder, nullptr);
    if (hyp)
    {
      std::istringstream st(hyp);
      std::string tmp;
      std::string phoneme = "SIL";
      while (std::getline(st, tmp, ' '))
      {
        if (tmp[0] != '+')
          phoneme = tmp;
      }
      using namespace std::literals;
      static auto const phonToViseme = std::unordered_map<std::string_view, Viseme>{
        {"AA"sv, Viseme::aa},
        {"AE"sv, Viseme::aa},
        {"AH"sv, Viseme::aa},
        {"AO"sv, Viseme::O},
        {"AW"sv, Viseme::O},
        {"AY"sv, Viseme::aa},
        {"B"sv, Viseme::PP},
        {"CH"sv, Viseme::CH},
        {"D"sv, Viseme::DD},
        {"DH"sv, Viseme::TH},
        {"EH"sv, Viseme::E},
        {"ER"sv, Viseme::E},
        {"EY"sv, Viseme::E},
        {"F"sv, Viseme::FF},
        {"G"sv, Viseme::kk},
        {"HH"sv, Viseme::CH},
        {"IH"sv, Viseme::I},
        {"IY"sv, Viseme::I},
        {"JH"sv, Viseme::CH},
        {"K"sv, Viseme::kk},
        {"L"sv, Viseme::nn},
        {"M"sv, Viseme::nn},
        {"N"sv, Viseme::nn},
        {"NG"sv, Viseme::nn},
        {"OW"sv, Viseme::O},
        {"OY"sv, Viseme::O},
        {"P"sv, Viseme::PP},
        {"R"sv, Viseme::RR},
        {"S"sv, Viseme::SS},
        {"SH"sv, Viseme::SS},
        {"SIL"sv, Viseme::sil},
        {"T"sv, Viseme::DD},
        {"TH"sv, Viseme::TH},
        {"UH"sv, Viseme::U},
        {"UW"sv, Viseme::U},
        {"V"sv, Viseme::FF},
        {"W"sv, Viseme::RR},
        {"Y"sv, Viseme::nn},
        {"Z"sv, Viseme::SS},
        {"ZH"sv, Viseme::SS},
      };

      auto it = phonToViseme.find(phoneme);
      if (it != std::end(phonToViseme))
      {
        for (auto v : sinks)
          v.get().ingest(it->second);
      }
      else
        SPDLOG_ERROR("Did not find phone mapping for: {}", phoneme);
    }
    if (!ps_endpointer_in_speech(ep))
    {
      ps_end_utt(decoder);
      ps_get_hyp(decoder, nullptr);
    }
  }
}

auto Wav2Visemes::sampleRate() const -> int
{
  return ps_endpointer_sample_rate(ep);
}

auto Wav2Visemes::frameSize() const -> int
{
  return static_cast<int>(ps_endpointer_frame_size(ep));
}

auto Wav2Visemes::reg(VisemesSink &v) -> void
{
  sinks.push_back(v);
}

auto Wav2Visemes::unreg(VisemesSink &v) -> void
{
  sinks.erase(
    std::remove_if(std::begin(sinks), std::end(sinks), [&](const auto &x) { return &x.get() == &v; }),
    std::end(sinks));
}
