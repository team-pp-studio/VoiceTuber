#include "azure-tts.hpp"
#include "audio-sink.hpp"
#include "azure-token.hpp"
#include "http-client.hpp"
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>

AzureTts::AzureTts(uv::Uv &uv,
                   AzureToken &azureToken,
                   class HttpClient &aHttpClient,
                   class AudioSink &aAudioSink)
  : timer(uv.createTimer()), token(azureToken), httpClient(aHttpClient), audioSink(aAudioSink)
{
  process();
}

static std::string escape(std::string data)
{
  // escape for XML
  std::string buffer;
  for (size_t pos = 0; pos != data.size(); ++pos)
  {
    switch (data[pos])
    {
    case '&': buffer.append("&amp;"); break;
    case '\"': buffer.append("&quot;"); break;
    case '\'': buffer.append("&apos;"); break;
    case '<': buffer.append("&lt;"); break;
    case '>': buffer.append("&gt;"); break;
    default: buffer.append(&data[pos], 1); break;
    }
  }
  return buffer;
}

auto AzureTts::say(std::string voice, std::string msg, bool overlap) -> void
{
  queue.emplace([msg = std::move(msg),
                 alive = weak_self(),
                 voice = std::move(voice),
                 overlap](std::string_view t, PostTask postTask) {
    if (auto self = alive.lock())
    {
      auto xml = R"(<speak version="1.0" xml:lang="en-us"><voice xml:lang="en-US" name=")" + voice +
                 R"("><prosody rate="0.00%">)" + escape(msg) + R"(</prosody></voice></speak>)";
      self->httpClient.get().post(
        "https://eastus.tts.speech.microsoft.com/cognitiveservices/v1",
        std::move(xml),
        [overlap, postTask = std::move(postTask), alive](
          CURLcode code, long httpStatus, std::string payload) mutable {
          if (auto self = alive.lock())
          {
            if (code != CURLE_OK)
            {
              postTask(false);
              return;
            }
            if (httpStatus == 401)
            {
              SPDLOG_INFO("{} {}", curl_easy_strerror(code), httpStatus);
              self->token.get().clear();
              postTask(false);
              return;
            }
            if (httpStatus >= 400 && httpStatus < 500)
            {
              SPDLOG_INFO("{} {} {}", curl_easy_strerror(code), httpStatus, payload);
              self->lastError = payload;
              postTask(true);
              return;
            }
            if (httpStatus != 200)
            {
              SPDLOG_INFO("{} {} {}", curl_easy_strerror(code), httpStatus, payload);
              self->lastError = payload;
              self->timer.start([postTask = std::move(postTask)]() mutable { postTask(false); }, 10'000);
              return;
            }

            self->lastError = "";
            Wav wav;
            const auto inF = 24000;
            const auto outF = self->audioSink.get().sampleRate();
            const auto inSz = static_cast<int>(payload.size() / sizeof(int16_t));
            const auto outSz = static_cast<int>(static_cast<int64_t>(inSz) * outF / inF);
            wav.resize(outSz);
            for (auto i = 0; i < outSz; ++i)
              wav[i] = reinterpret_cast<int16_t *>(payload.data())[static_cast<int64_t>(i) * inF / outF];
            self->audioSink.get().ingest(std::move(wav), overlap);
            postTask(true);
          }
          else

          {
            SPDLOG_INFO("this was destroyed");
          }
        },
        {{"Accept", ""},
         {"User-Agent", "curl/7.68.0"},
         {"Authorization", fmt::format("Bearer {}", t)},
         {"Content-Type", "application/ssml+xml"},
         {"X-Microsoft-OutputFormat", "raw-24khz-16bit-mono-pcm"}});
    }
    else

    {
      SPDLOG_INFO("this was destroyed");
    }
  });
  process();
}

auto AzureTts::process() -> void
{
  if (state == State::waiting)
    return;
  if (queue.empty())
  {
    state = State::idle;
    return;
  }
  state = State::waiting;
  token.get().get([alive = weak_self()](const std::string &t, const std::string &err) {
    if (auto self = alive.lock())
    {

      if (t.empty())
      {
        self->lastError = err;
        self->timer.start([alive]() {
          if (auto self = alive.lock())
          {
            self->state = State::idle;
            self->process();
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        });
        return;
      }
      self->queue.front()(t, [alive](bool r) {
        if (auto self = alive.lock())
        {
          self->timer.start([alive, r]() {
            if (auto self = alive.lock())
            {
              if (r)
                self->queue.pop();
              self->state = State::idle;
              self->process();
            }
            else
            {
              SPDLOG_INFO("this was destroyed");
            }
          });
        }
        else
        {
          SPDLOG_INFO("this was destroyed");
          return;
        }
      });
    }
    else
    {
      SPDLOG_INFO("this was destroyed");
    }
  });
}

auto AzureTts::listVoices(ListVoicesCallback cb) -> void
{
  queue.emplace([cb = std::move(cb), alive = weak_self()](std::string_view t, PostTask postTask) mutable {
    if (auto self = alive.lock())
    {
      self->httpClient.get().get(
        "https://eastus.tts.speech.microsoft.com/cognitiveservices/voices/list",
        [cb = std::move(cb), postTask = std::move(postTask), alive](
          CURLcode code, long httpStatus, std::string payload) mutable {
          if (auto self = alive.lock())
          {
            if (code != CURLE_OK)
            {
              cb({});
              postTask(false);
              return;
            }
            if (httpStatus == 401)
            {
              SPDLOG_INFO("{} {}", curl_easy_strerror(code), httpStatus);
              self->token.get().clear();
              cb({});
              postTask(false);
              return;
            }
            if (httpStatus != 200)
            {
              SPDLOG_INFO("{} {} {}", curl_easy_strerror(code), httpStatus, payload);
              self->lastError = payload;
              cb({});
              postTask(false);
              return;
            }

            rapidjson::Document document;
            document.Parse(payload.data(), payload.size());
            std::vector<std::string_view> voices;
            for (auto const &element : document.GetArray())
            {
              //[
              //  {
              //    "Name": "Microsoft Server Speech Text to Speech Voice (af-ZA, AdriNeural)",
              //    "DisplayName": "Adri",
              //    "LocalName": "Adri",
              //    "ShortName": "af-ZA-AdriNeural",
              //    "Gender": "Female",
              //    "Locale": "af-ZA",
              //    "LocaleName": "Afrikaans (South Africa)",
              //    "SampleRateHertz": "48000",
              //    "VoiceType": "Neural",
              //    "Status": "GA",
              //    "WordsPerMinute": "147"
              //  },
              //  {
              //    "Name": "Microsoft Server Speech Text to Speech Voice (af-ZA, WillemNeural)",
              //    "DisplayName": "Willem",
              //    "LocalName": "Willem",
              //    "ShortName": "af-ZA-WillemNeural",
              //    "Gender": "Male",
              //    "Locale": "af-ZA",
              //    "LocaleName": "Afrikaans (South Africa)",
              //    "SampleRateHertz": "48000",
              //    "VoiceType": "Neural",
              //    "Status": "GA",
              //    "WordsPerMinute": "155"
              //  },
              //  {
              //    "Name": "Microsoft Server Speech Text to Speech Voice (am-ET, AmehaNeural)",
              //    "DisplayName": "Ameha",
              //    "LocalName": "አምሀ",
              //    "ShortName": "am-ET-AmehaNeural",
              //    "Gender": "Male",
              //    "Locale": "am-ET",
              //    "LocaleName": "Amharic (Ethiopia)",
              //    "SampleRateHertz": "48000",
              //    "VoiceType": "Neural",
              //    "Status": "GA",
              //    "WordsPerMinute": "112"
              //  }, ...

              const auto locale = std::string_view{element["Locale"].GetString(), element["Locale"].GetStringLength()};
              if (locale.starts_with("en-") != 0)
                continue;
              voices.emplace_back(std::string_view{element["ShortName"].GetString(), element["ShortName"].GetStringLength()});
            }
            self->lastError.clear();

            cb(voices);
            postTask(true);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        },
        {
          {"Accept", ""},
          {"User-Agent", "curl/7.68.0"},
          {"Authorization", fmt::format("Bearer {}", t)},
        });
    }
    else
    {
      SPDLOG_INFO("this was destroyed");
    }
  });
  process();
}
