#include "azure-token.hpp"
#include "http-client.hpp"
#include <log/log.hpp>

AzureToken::AzureToken(std::string aKey, class HttpClient &aHttpClient)
  : key(std::move(aKey)), httpClient(aHttpClient)
{
}

auto AzureToken::get(Callback cb) -> void
{
  callbacks.emplace_back(std::move(cb));
  if (!token.empty())
  {
    for (auto &lCb : callbacks)
      lCb(token, "");
    callbacks.clear();
    return;
  }
  httpClient.get().post(
    "https://eastus.api.cognitive.microsoft.com/sts/v1.0/issuetoken",
    "",
    [alive = this->weak_from_this()](CURLcode code, long httpStatus, std::string payload) {
      if (auto self = alive.lock())
      {
        if (code != CURLE_OK)
        {
          LOG(code, httpStatus, payload);
          for (auto &lCb : self->callbacks)
            lCb("", std::string{"CURL Error: "} + curl_easy_strerror(code));
          self->callbacks.clear();
          return;
        }
        if (httpStatus != 200)
        {
          LOG(code, httpStatus, payload);
          for (auto &lCb : self->callbacks)
            lCb("", "HTTP Status: " + std::to_string(httpStatus) + " " + payload);
          self->callbacks.clear();
          return;
        }
        self->token = std::move(payload);
        for (auto &lCb : self->callbacks)
          lCb(self->token, "");
        self->callbacks.clear();
      }
      else

      {
        LOG("this was destroyed");
      }
    },
    {{"Ocp-Apim-Subscription-Key", key}, {"Expect", ""}});
}

auto AzureToken::clear() -> void
{
  token.clear();
}

auto AzureToken::updateKey(const std::string &k) -> void
{
  if (k == key)
    return;
  key = k;
  token.clear();
}
