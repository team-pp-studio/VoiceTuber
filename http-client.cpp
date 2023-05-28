#include "http-client.hpp"
#include <algorithm>
#include <log/log.hpp>

namespace
{
  class CurlInitializer
  {
  public:
    CurlInitializer()
    {
      if (curl_global_init(CURL_GLOBAL_ALL))
        throw std::runtime_error("Could not init curl");
    }
    static auto init() -> void { static auto curlInit = CurlInitializer{}; }
  };

} // namespace

HttpClient::HttpClient(class Uv &aUv) : uv(aUv), timeout(aUv.getTimer()), multiHandle(curl_multi_init())
{
  CurlInitializer::init();
#pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
  curl_multi_setopt(multiHandle, CURLMOPT_SOCKETDATA, this);
  curl_multi_setopt(multiHandle, CURLMOPT_SOCKETFUNCTION, &HttpClient::socketFunc_);
  curl_multi_setopt(multiHandle, CURLMOPT_TIMERDATA, this);
  curl_multi_setopt(multiHandle, CURLMOPT_TIMERFUNCTION, HttpClient::startTimeout_);
}

auto HttpClient::startTimeout_(CURLM *, long timeout_ms, void *userp) -> int
{
  return static_cast<HttpClient *>(userp)->startTimeout(timeout_ms);
}

auto HttpClient::socketFunc_(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp) -> int
{
  return static_cast<HttpClient *>(userp)->socketFunc(easy, s, action, socketp);
}

HttpClient::~HttpClient()
{
  // TODO-Mika cleanup curl_multi_cleanup(multiHandle);
}

auto HttpClient::socketFunc(CURL *, curl_socket_t s, int action, void *socketp) -> int
{
  switch (action)
  {
  case CURL_POLL_IN:
  case CURL_POLL_OUT:
  case CURL_POLL_INOUT: {
    int events = 0;
    auto sockContext = socketp ? static_cast<SockContext *>(socketp) : createSockContext(s);
    curl_multi_assign(multiHandle, s, sockContext);

    if (action != CURL_POLL_IN)
      events |= UV_WRITABLE;
    if (action != CURL_POLL_OUT)
      events |= UV_READABLE;

    uv_poll_start(&sockContext->pollHandle, events, [](uv_poll_t *req, int status, int aEvents) {
      static_cast<SockContext *>(req->data)->self->curlPerform(req, status, aEvents);
    });
    break;
  }
  case CURL_POLL_REMOVE:
    if (!socketp)
      break;
    uv_poll_stop(&((SockContext *)socketp)->pollHandle);
    destroySockContext(static_cast<SockContext *>(socketp));
    curl_multi_assign(multiHandle, s, NULL);
    break;
  default: abort();
  }

  return 0;
}

auto HttpClient::createSockContext(curl_socket_t sockfd) -> SockContext *
{
  SockContext *context;
  context = new SockContext;
  context->self = this;
  context->sockfd = sockfd;
  uv_poll_init_socket(uv.get().loop(), &context->pollHandle, sockfd);
  context->pollHandle.data = context;
  return context;
}

void HttpClient::destroySockContext(SockContext *context)
{
  uv_close((uv_handle_t *)&context->pollHandle,
           [](auto handle) { delete static_cast<SockContext *>(handle->data); });
}

auto HttpClient::checkMultiInfo() -> void
{
  int pending;

  while (auto message = curl_multi_info_read(multiHandle, &pending))
  {
    switch (message->msg)
    {
    case CURLMSG_DONE: {
      /* Do not use message data after calling curl_multi_remove_handle() and
         curl_easy_cleanup(). As per curl_multi_info_read() docs:
         "WARNING: The data the returned pointer points to will not survive
         calling curl_multi_cleanup, curl_multi_remove_handle or
         curl_easy_cleanup." */
      auto easyHandle = message->easy_handle;

      char *doneUrl;
      curl_easy_getinfo(easyHandle, CURLINFO_EFFECTIVE_URL, &doneUrl);
      CurlContext *ctx;
      curl_easy_getinfo(easyHandle, CURLINFO_PRIVATE, &ctx);
      long codep;
      curl_easy_getinfo(easyHandle, CURLINFO_RESPONSE_CODE, &codep);
      ctx->cb(message->data.result, codep, std::move(ctx->payloadOut));
      curl_slist_free_all(ctx->headers);
      delete ctx;
      curl_multi_remove_handle(multiHandle, easyHandle);
      curl_easy_cleanup(easyHandle);
      break;
    }
    case CURLMSG_NONE:
    case CURLMSG_LAST: fprintf(stderr, "CURLMSG default\n"); break;
    }
  }
}

void HttpClient::curlPerform(uv_poll_t *req, int /*status*/, int events)
{
  auto context = static_cast<SockContext *>(req->data);

  int flags = 0;
  if (events & UV_READABLE)
    flags |= CURL_CSELECT_IN;
  if (events & UV_WRITABLE)
    flags |= CURL_CSELECT_OUT;

  int runningHandles;
  curl_multi_socket_action(multiHandle, context->sockfd, flags, &runningHandles);

  checkMultiInfo();
}

auto HttpClient::onTimeout() -> void
{
  int runningHandles;
  curl_multi_socket_action(multiHandle, CURL_SOCKET_TIMEOUT, 0, &runningHandles);
  checkMultiInfo();
}

auto HttpClient::startTimeout(long timeoutMs) -> int
{
  if (timeoutMs < 0)
    timeout.stop();
  else
    timeout.start([this]() { onTimeout(); }, timeoutMs == 0 ? 1 : timeoutMs, 0);
  return 0;
}

auto HttpClient::get(const std::string &url, Cb cb, const Headers &headers) -> void
{
  auto handle = curl_easy_init();
  auto ctx = new CurlContext;
  ctx->self = this;
  ctx->cb = std::move(cb);
  curl_easy_setopt(handle, CURLOPT_PRIVATE, ctx);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, ctx);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, CurlContext::write_);
  curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
#ifdef _WIN32
  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
  if (!headers.empty())
  {
    for (const auto &h : headers)
      ctx->headers = curl_slist_append(
        ctx->headers, (h.first + ":" + (!h.second.empty() ? (" " + h.second) : "")).c_str());
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, ctx->headers);
  }
  curl_multi_add_handle(multiHandle, handle);
}

auto HttpClient::CurlContext::write_(char *in, unsigned size, unsigned nmemb, void *ctx) -> size_t
{
  return static_cast<CurlContext *>(ctx)->write(in, size, nmemb);
}

auto HttpClient::CurlContext::write(char *in, unsigned size, unsigned nmemb) -> size_t
{
  payloadOut += std::string_view{in, size * nmemb};
  return size * nmemb;
}

auto HttpClient::CurlContext::read(char *out, unsigned size, unsigned nmemb) -> size_t
{
  const auto ret = std::min(static_cast<unsigned>(payloadOut.size()), size * nmemb);
  std::copy(std::begin(payloadOut), std::begin(payloadOut) + ret, out);
  payloadOut.erase(0, ret);
  return ret;
}

auto HttpClient::CurlContext::read_(char *out, unsigned size, unsigned nmemb, void *ctx) -> size_t
{
  return static_cast<CurlContext *>(ctx)->read(out, size, nmemb);
}

auto HttpClient::post(const std::string &url, std::string post, Cb cb, const Headers &headers) -> void
{
  auto handle = curl_easy_init();
  auto ctx = new CurlContext;
  ctx->self = this;
  ctx->cb = std::move(cb);
  ctx->payloadOut = std::move(post);
  curl_easy_setopt(handle, CURLOPT_PRIVATE, ctx);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, ctx);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, CurlContext::write_);
  curl_easy_setopt(handle, CURLOPT_READDATA, ctx);
  curl_easy_setopt(handle, CURLOPT_READFUNCTION, CurlContext::read_);
  curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
  curl_easy_setopt(handle, CURLOPT_POST, 1L);
#ifdef _WIN32
  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
  if (!headers.empty())
  {
    for (const auto &h : headers)

      ctx->headers = curl_slist_append(
        ctx->headers, (h.first + ":" + (!h.second.empty() ? (" " + h.second) : "")).c_str());
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, ctx->headers);
  }

  curl_multi_add_handle(multiHandle, handle);
}
