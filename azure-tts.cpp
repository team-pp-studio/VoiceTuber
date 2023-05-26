#include "azure-tts.hpp"
#include "audio-sink.hpp"
#include "http-client.hpp"
#include <log/log.hpp>

AzureTts::AzureTts(std::string aKey, class HttpClient &aHttpClient, class AudioSink &aAudioSink)
  : key(aKey), httpClient(aHttpClient), audioSink(aAudioSink)
{
  LOG("ctor AzureTts");
  process();
}

AzureTts::~AzureTts()
{
  LOG("dtor AzureTts");
}

auto AzureTts::say(std::string voice, std::string msg) -> void
{
  queue.emplace(std::move(voice), std::move(msg));
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
  auto [voice, msg] = queue.front();
  auto xml = R"(<speak version="1.0" xml:lang="en-us"><voice xml:lang="en-US" name=")" + voice +
             R"("><prosody rate="0.00%">)" + escape(msg) + R"(</prosody></voice></speak>)";
  httpClient.get().post("https://eastus.tts.speech.microsoft.com/cognitiveservices/v1",
                        std::move(xml),
                        [this](CURLcode code, long httpStatus, std::string payload) {
                          state = State::idle;
                          if (code != CURLE_OK)
                            return;
                          if (httpStatus == 401)
                          {
                            LOG(code, httpStatus, payload);
                            token.clear();
                            return;
                          }
                          if (httpStatus != 200)
                          {
                            LOG(code, httpStatus, payload);
                            return;
                          }
                          queue.pop();

                          Wav wav;
                          const auto inF = 24000;
                          const auto outF = audioSink.get().sampleRate();
                          const auto inSz = static_cast<int>(payload.size() / sizeof(int16_t));
                          const auto outSz = static_cast<int>(static_cast<int64_t>(inSz) * outF / inF);
                          LOG("inF", inF, "outF", outF, "inSz", inSz, "outSz", outSz);
                          wav.resize(outSz);
                          for (auto i = 0; i < outSz; ++i)
                            wav[i] = reinterpret_cast<int16_t *>(payload.data())[static_cast<int64_t>(i) * inF / outF];
                          audioSink.get().ingest(std::move(wav));
                          process();
                        },
                        {{"Accept", ""},
                         {"User-Agent", "curl/7.68.0"},
                         {"Authorization", "Bearer " + token},
                         {"Content-Type", "application/ssml+xml"},
                         {"X-Microsoft-OutputFormat", "raw-24khz-16bit-mono-pcm"}});
}

auto AzureTts::getToken() -> void
{
  httpClient.get().post("https://eastus.api.cognitive.microsoft.com/sts/v1.0/issuetoken",
                        "",
                        [this](CURLcode code, long httpStatus, std::string payload) {
                          state = State::idle;
                          if (code != CURLE_OK)
                          {
                            LOG(code, httpStatus, payload);
                            return;
                          }
                          if (httpStatus != 200)
                          {
                            LOG(code, httpStatus, payload);
                            return;
                          }
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
