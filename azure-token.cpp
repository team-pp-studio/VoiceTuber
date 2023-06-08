#include "azure-token.hpp"
#include "http-client.hpp"
#include <log/log.hpp>

AzureToken::AzureToken(std::string aKey, class HttpClient &aHttpClient)
  : key(std::move(aKey)), httpClient(aHttpClient)
{
}

auto AzureToken::get(Cb cb) -> void
{
  cbs.emplace_back(std::move(cb));
  if (!token.empty())
  {
    for (const auto &lCb : cbs)
      lCb(token, "");
    cbs.clear();
    return;
  }
  httpClient.get().post("https://eastus.api.cognitive.microsoft.com/sts/v1.0/issuetoken",
                        "",
                        [this](CURLcode code, long httpStatus, std::string payload) {
                          if (code != CURLE_OK)
                          {
                            LOG(code, httpStatus, payload);
                            for (const auto &lCb : cbs)
                              lCb("", std::string{"CURL Error: "} + curl_easy_strerror(code));
                            cbs.clear();
                            return;
                          }
                          if (httpStatus != 200)
                          {
                            LOG(code, httpStatus, payload);
                            for (const auto &lCb : cbs)
                              lCb("", "HTTP Status: " + std::to_string(httpStatus) + " " + payload);
                            cbs.clear();
                            return;
                          }
                          token = std::move(payload);
                          for (const auto &lCb : cbs)
                            lCb(token, "");
                          cbs.clear();
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
