#include "azure-stt.hpp"

#include <cmath>
#include <sstream>

#include <fmt/std.h>
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>

#include "azure-token.hpp"
#include "http-client.hpp"
#include "save-wav.hpp"

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
        }
      });
    }
    else

    {
      SPDLOG_INFO("this was destroyed");
    }
  });
}

auto AzureStt::perform(Wav wav, int sampleRate, Callback cb) -> void
{
  auto dur = 1.f * wav.size() / sampleRate;
  total += dur;
  SPDLOG_INFO("Azure {} seconds, total: {} minutes {} seconds", dur, std::floor(total / 60.f), static_cast<int>(total) % 60);
  queue.emplace([cb = std::move(cb), sampleRate, alive = weak_self(), wav = std::move(wav)](
                  const std::string &t, PostTask postTask) mutable {
    if (auto self = alive.lock())
    {
      std::ostringstream ss;
      saveWav(ss, wav, sampleRate);
      self->httpClient.get().post(
        "https://eastus.stt.speech.microsoft.com/speech/recognition/conversation/cognitiveservices/"
        "v1?language=en-US",
        ss.str(),
        [cb = std::move(cb), postTask = std::move(postTask), alive](
          CURLcode code, long httpStatus, std::string payload) mutable {
          if (auto self = alive.lock())
          {
            if (code != CURLE_OK)
            {
              cb("");
              postTask(false);
              return;
            }
            if (httpStatus == 401)
            {
              SPDLOG_INFO("{} {}", curl_easy_strerror(code), httpStatus);
              self->token.get().clear();
              cb("");
              postTask(false);
              return;
            }
            if (httpStatus >= 400 && httpStatus < 500)
            {
              SPDLOG_INFO("{} {} {}", curl_easy_strerror(code), httpStatus, payload);
              self->lastError = payload;
              cb("");
              postTask(true);
              return;
            }
            if (httpStatus != 200)
            {
              SPDLOG_INFO("{} {} {}", curl_easy_strerror(code), httpStatus, payload);
              self->lastError = payload;
              cb("");
              self->timer.start([postTask = std::move(postTask)]() mutable { postTask(false); }, 10'000);
              return;
            }

            self->lastError = "";
            rapidjson::Document document;
            document.Parse(payload.data(), payload.size());
            // {"RecognitionStatus":"Success","Offset":600000,"Duration":30000000,"DisplayText":"What do you think about it?"}
            auto const &display_text = document["DisplayText"];
            cb(std::string_view{display_text.GetString(), display_text.GetStringLength()});
            postTask(true);
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
          }
        },
        {{"Accept", ""},
         {"User-Agent", "curl/7.68.0"},
         {"Authorization", "Bearer " + t},
         {"Content-Type", "audio/wav"}});
    }
    else

    {
      SPDLOG_INFO("this was destroyed");
    }
  });
  process();
}
