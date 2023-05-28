#include "azure-tts.hpp"
#include "audio-sink.hpp"
#include "http-client.hpp"
#include <json/json.hpp>
#include <log/log.hpp>

AzureTts::AzureTts(Uv &uv, std::string aKey, class HttpClient &aHttpClient, class AudioSink &aAudioSink)
  : timer(uv.getTimer()), key(aKey), httpClient(aHttpClient), audioSink(aAudioSink)
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

auto AzureTts::say(std::string voice, std::string msg) -> void
{
  queue.emplace([msg = std::move(msg), this, voice = std::move(voice)](PostTask postTask) {
    auto xml = R"(<speak version="1.0" xml:lang="en-us"><voice xml:lang="en-US" name=")" + voice +
               R"("><prosody rate="0.00%">)" + escape(msg) + R"(</prosody></voice></speak>)";
    httpClient.get().post(
      "https://eastus.tts.speech.microsoft.com/cognitiveservices/v1",
      std::move(xml),
      [postTask = std::move(postTask), this](CURLcode code, long httpStatus, std::string payload) {
        if (code != CURLE_OK)
        {
          postTask(false);
          return;
        }
        if (httpStatus == 401)
        {
          LOG(code, httpStatus, payload);
          token.clear();
          postTask(false);
          return;
        }
        if (httpStatus >= 400 && httpStatus < 500)
        {
          LOG(code, httpStatus, payload);
          lastError = payload;
          postTask(true);
          return;
        }
        if (httpStatus != 200)
        {
          LOG(code, httpStatus, payload);
          lastError = payload;
          timer.start([postTask = std::move(postTask)]() { postTask(false); }, 10'000);
          return;
        }

        lastError = "";
        Wav wav;
        const auto inF = 24000;
        const auto outF = audioSink.get().sampleRate();
        const auto inSz = static_cast<int>(payload.size() / sizeof(int16_t));
        const auto outSz = static_cast<int>(static_cast<int64_t>(inSz) * outF / inF);
        wav.resize(outSz);
        for (auto i = 0; i < outSz; ++i)
          wav[i] = reinterpret_cast<int16_t *>(payload.data())[static_cast<int64_t>(i) * inF / outF];
        audioSink.get().ingest(std::move(wav));
        postTask(true);
      },
      {{"Accept", ""},
       {"User-Agent", "curl/7.68.0"},
       {"Authorization", "Bearer " + token},
       {"Content-Type", "application/ssml+xml"},
       {"X-Microsoft-OutputFormat", "raw-24khz-16bit-mono-pcm"}});
  });
  process();
}

auto AzureTts::process() -> void
{
  if (state == State::waiting)
    return;
  if (token.empty())
  {
    getToken();
    return;
  }
  if (queue.empty())
  {
    state = State::idle;
    return;
  }
  state = State::waiting;
  queue.front()([this](bool r) {
    timer.start([this, r]() {
      if (r)
        queue.pop();
      state = State::idle;
      process();
    });
  });
}

auto AzureTts::getToken() -> void
{
  state = State::waiting;
  httpClient.get().post("https://eastus.api.cognitive.microsoft.com/sts/v1.0/issuetoken",
                        "",
                        [this](CURLcode code, long httpStatus, std::string payload) {
                          state = State::idle;
                          if (code != CURLE_OK)
                          {
                            LOG(code, httpStatus, payload);
                            lastError = payload;
                            return;
                          }
                          if (httpStatus != 200)
                          {
                            LOG(code, httpStatus, payload);
                            lastError = payload;
                            return;
                          }
                          lastError = "";
                          token = std::move(payload);
                          process();
                        },
                        {{"Ocp-Apim-Subscription-Key", key}, {"Expect", ""}});
}

auto AzureTts::updateKey(std::string aKey) -> void
{
  if (key == aKey)
    return;
  key = std::move(aKey);
  token.clear();
  process();
}

auto AzureTts::listVoices(ListVoicesCb cb) -> void
{
  queue.emplace([cb = std::move(cb), this](PostTask postTask) {
    httpClient.get().get(
      "https://eastus.tts.speech.microsoft.com/cognitiveservices/voices/list",
      [cb = std::move(cb), postTask = std::move(postTask), this](
        CURLcode code, long httpStatus, std::string payload) {
        if (code != CURLE_OK)
        {
          cb({});
          postTask(false);
          return;
        }
        if (httpStatus == 401)
        {
          LOG(code, httpStatus, payload);
          token.clear();
          cb({});
          postTask(false);
          return;
        }
        if (httpStatus != 200)
        {
          LOG(code, httpStatus, payload);
          lastError = payload;
          cb({});
          postTask(false);
          return;
        }

        const auto list = json::Root{std::move(payload)};
        LOG("Voices num", list.size());
        std::vector<std::string> voices;
        for (auto i = 0U; i < list.size(); ++i)
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

          const auto obj = list[i];
          const auto locale = obj("Locale").asStr();
          if (locale.find("en-") != 0)
            continue;
          voices.emplace_back(obj("ShortName").asStr());
        }
        lastError = "";

        cb(std::move(voices));
        postTask(true);
      },
      {{"Accept", ""}, {"User-Agent", "curl/7.68.0"}, {"Authorization", "Bearer " + token}});
  });
  process();
}
