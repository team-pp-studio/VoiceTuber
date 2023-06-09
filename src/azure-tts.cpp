#include "azure-tts.hpp"
#include "audio-sink.hpp"
#include "azure-token.hpp"
#include "http-client.hpp"
#include <json/json.hpp>
#include <log/log.hpp>

AzureTts::AzureTts(uv::Uv &uv,
                   AzureToken &azureToken,
                   class HttpClient &aHttpClient,
                   class AudioSink &aAudioSink)
  : alive(std::make_shared<int>()),
    timer(uv.createTimer()),
    token(azureToken),
    httpClient(aHttpClient),
    audioSink(aAudioSink)
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
  queue.emplace(
    [msg = std::move(msg), alive = std::weak_ptr<int>(alive), this, voice = std::move(voice), overlap](
      const std::string &t, PostTask postTask) {
      if (!alive.lock())
      {
        LOG("this was destroyed");
        return;
      }
      auto xml = R"(<speak version="1.0" xml:lang="en-us"><voice xml:lang="en-US" name=")" + voice +
                 R"("><prosody rate="0.00%">)" + escape(msg) + R"(</prosody></voice></speak>)";
      httpClient.get().post(
        "https://eastus.tts.speech.microsoft.com/cognitiveservices/v1",
        std::move(xml),
        [overlap, postTask = std::move(postTask), alive = std::weak_ptr<int>(alive), this](
          CURLcode code, long httpStatus, std::string payload) {
          if (!alive.lock())
          {
            LOG("this was destroyed");
            return;
          }
          if (code != CURLE_OK)
          {
            postTask(false);
            return;
          }
          if (httpStatus == 401)
          {
            LOG(code, httpStatus);
            token.get().clear();
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
          audioSink.get().ingest(std::move(wav), overlap);
          postTask(true);
        },
        {{"Accept", ""},
         {"User-Agent", "curl/7.68.0"},
         {"Authorization", "Bearer " + t},
         {"Content-Type", "application/ssml+xml"},
         {"X-Microsoft-OutputFormat", "raw-24khz-16bit-mono-pcm"}});
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
  token.get().get(
    [alive = std::weak_ptr<int>(alive), this](const std::string &t, const std::string &err) {
      if (!alive.lock())
      {
        LOG("this was destroyed");
        return;
      }
      if (t.empty())
      {
        lastError = err;
        timer.start([alive = std::weak_ptr<int>(alive), this]() {
          if (!alive.lock())
          {
            LOG("this was destroyed");
            return;
          }
          state = State::idle;
          process();
        });
        return;
      }
      queue.front()(t, [alive = std::weak_ptr<int>(alive), this](bool r) {
        if (!alive.lock())
        {
          LOG("this was destroyed");
          return;
        }
        timer.start([alive = std::weak_ptr<int>(alive), this, r]() {
          if (!alive.lock())
          {
            LOG("this was destroyed");
            return;
          }
          if (r)
            queue.pop();
          state = State::idle;
          process();
        });
      });
    });
}

auto AzureTts::listVoices(ListVoicesCb cb) -> void
{
  queue.emplace([cb = std::move(cb), alive = std::weak_ptr<int>(alive), this](const std::string &t,
                                                                              PostTask postTask) {
    if (!alive.lock())
    {
      LOG("this was destroyed");
      return;
    }
    httpClient.get().get(
      "https://eastus.tts.speech.microsoft.com/cognitiveservices/voices/list",
      [cb = std::move(cb), postTask = std::move(postTask), alive = std::weak_ptr<int>(alive), this](
        CURLcode code, long httpStatus, std::string payload) {
        if (!alive.lock())
        {
          LOG("this was destroyed");
          return;
        }
        if (code != CURLE_OK)
        {
          cb({});
          postTask(false);
          return;
        }
        if (httpStatus == 401)
        {
          LOG(code, httpStatus);
          token.get().clear();
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
      {{"Accept", ""}, {"User-Agent", "curl/7.68.0"}, {"Authorization", "Bearer " + t}});
  });
  process();
}
