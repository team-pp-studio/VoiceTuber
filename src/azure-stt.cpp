#include "azure-stt.hpp"
#include "azure-token.hpp"
#include "http-client.hpp"
#include "save-wav.hpp"
#include <cmath>
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
  token.get().get([alive = this->weak_from_this()](const std::string &t, const std::string &err) {
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
            LOG("this was destroyed");
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
              LOG("this was destroyed");
            }
          });
        }
        else
        {
          LOG("this was destroyed");
        }
      });
    }
    else

    {
      LOG("this was destroyed");
    }
  });
}

auto AzureStt::perform(Wav wav, int sampleRate, Callback cb) -> void
{
  auto dur = 1.f * wav.size() / sampleRate;
  total += dur;
  LOG("Azure",
      dur,
      "seconds Total:",
      std::floor(total / 60),
      "minuts",
      static_cast<int>(total) % 60,
      "seconds");
  queue.emplace([cb = std::move(cb), sampleRate, alive = this->weak_from_this(), wav = std::move(wav)](
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
              LOG(code, httpStatus);
              self->token.get().clear();
              cb("");
              postTask(false);
              return;
            }
            if (httpStatus >= 400 && httpStatus < 500)
            {
              LOG(code, httpStatus, payload);
              self->lastError = payload;
              cb("");
              postTask(true);
              return;
            }
            if (httpStatus != 200)
            {
              LOG(code, httpStatus, payload);
              self->lastError = payload;
              cb("");
              self->timer.start([postTask = std::move(postTask)]() mutable { postTask(false); }, 10'000);
              return;
            }

            self->lastError = "";
            const auto j = json::Root{std::move(payload)};
            // {"RecognitionStatus":"Success","Offset":600000,"Duration":30000000,"DisplayText":"What
            // do you think about it?"}
            cb(j("DisplayText").asStr());
            postTask(true);
          }
          else
          {
            LOG("this was destroyed");
          }
        },
        {{"Accept", ""},
         {"User-Agent", "curl/7.68.0"},
         {"Authorization", "Bearer " + t},
         {"Content-Type", "audio/wav"}});
    }
    else

    {
      LOG("this was destroyed");
    }
  });
  process();
}
