#pragma once
#include <functional>
#include <string>
#include <utility>
#include <vector>

#define NOMINMAX
#define CURL_STATICLIB
#include <curl/curl.h>

class HttpClient
{
public:
  using Cb = std::function<auto(CURLcode, int httpStatus, std::string payload)->void>;
  auto get(const std::string &url,
           const std::vector<std::pair<std::string, std::string>> &headers,
           Cb cb);
  auto post(const std::string &url,
            const std::vector<std::pair<std::string, std::string>> &chunks,
            Cb cb);
};
