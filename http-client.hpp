#pragma once
#include "uv.hpp"
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
  using Headers = std::vector<std::pair<std::string, std::string>>;
  using Cb = std::function<auto(CURLcode, long httpStatus, std::string payload)->void>;

  HttpClient(class Uv &);
  HttpClient(const HttpClient &) = delete;
  ~HttpClient();
  auto get(const std::string &url, Cb cb, const Headers &headers = Headers{}) -> void;
  auto post(const std::string &url, Cb cb, const Headers &chunks = Headers{}) -> void;

private:
  std::reference_wrapper<Uv> uv;
  Timer timeout;
  CURLM *multiHandle = nullptr;
  struct SockContext
  {
    HttpClient *self;
    uv_poll_t pollHandle;
    curl_socket_t sockfd;
  };
  struct CurlContext
  {
    HttpClient *self;
    std::string payload;
    curl_slist *headers = nullptr;
    Cb cb;
    auto write(char *in, unsigned size, unsigned nmemb) -> size_t;
    static auto write_(char *in, unsigned size, unsigned nmemb, void *ctx) -> size_t;
    auto done() -> void;
  };
  auto checkMultiInfo() -> void;
  auto createSockContext(curl_socket_t sockfd) -> SockContext *;
  auto curlPerform(uv_poll_t *req, int status, int events) -> void;
  auto destroySockContext(SockContext *context) -> void;
  auto onTimeout() -> void;
  auto socketFunc(CURL *easy, curl_socket_t s, int action, void *socketp) -> int;
  auto startTimeout(long timeoutMs) -> int;
  static auto socketFunc_(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp) -> int;
  static auto startTimeout_(CURLM *, long timeout_ms, void *userp) -> int;
};
