#include "azure-stt.hpp"
#include "azure-token.hpp"
#include "http-client.hpp"
#include "save-wav.hpp"
#include <json/json.hpp>
#include <log/log.hpp>
#include <sstream>

AzureStt::AzureStt(uv::Uv &uv, AzureToken &aToken, HttpClient &aHttpClient)
  : timer(uv.createTimer()), token(aToken), httpClient(aHttpClient)
{
  process();
}

auto AzureStt::process() -> void
{
  if (state == State::waiting)
    return;
  if (queue.empty())
  {
    state = State::idle;
    return;
  }
  state = State::waiting;
  token.get().get([this](const std::string &t, const std::string &err) {
    if (t.empty())
    {
      lastError = err;
      timer.start([this]() {
        state = State::idle;
        process();
      });
      return;
    }
    queue.front()(t, [this](bool r) {
      timer.start([this, r]() {
        if (r)
          queue.pop();
        state = State::idle;
        process();
      });
    });
  });
}

auto AzureStt::perform(Wav wav, int sampleRate, Cb cb) -> void
{
  queue.emplace([cb = std::move(cb), sampleRate, this, wav = std::move(wav)](const std::string &t,
                                                                             PostTask postTask) {
    std::ostringstream ss;
    saveWav(ss, wav, sampleRate);
    httpClient.get().post("https://eastus.stt.speech.microsoft.com/speech/recognition/conversation/"
                          "cognitiveservices/v1?language=en-US",
                          ss.str(),
                          [cb = std::move(cb), postTask = std::move(postTask), this](
                            CURLcode code, long httpStatus, std::string payload) {
                            if (code != CURLE_OK)
                            {
                              cb("");
                              postTask(false);
                              return;
                            }
                            if (httpStatus == 401)
                            {
                              LOG(code, httpStatus, payload);
                              token.get().clear();
                              cb("");
                              postTask(false);
                              return;
                            }
                            if (httpStatus >= 400 && httpStatus < 500)
                            {
                              LOG(code, httpStatus, payload);
                              lastError = payload;
                              cb("");
                              postTask(true);
                              return;
                            }
                            if (httpStatus != 200)
                            {
                              LOG(code, httpStatus, payload);
                              lastError = payload;
                              cb("");
                              timer.start([postTask = std::move(postTask)]() { postTask(false); },
                                          10'000);
                              return;
                            }

                            lastError = "";
                            const auto j = json::Root{std::move(payload)};
                            // {"RecognitionStatus":"Success","Offset":600000,"Duration":30000000,"DisplayText":"What
                            // do you think about it?"}
                            cb(j("DisplayText").asStr());
                            postTask(true);
                          },
                          {{"Accept", ""},
                           {"User-Agent", "curl/7.68.0"},
                           {"Authorization", "Bearer " + t},
                           {"Content-Type", "audio/wav"}});
  });
  process();
}
