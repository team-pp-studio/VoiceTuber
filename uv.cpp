#include "uv.hpp"
#include <log/log.hpp>
#include <sstream>
#include <stdexcept>

Tcp::Tcp(uv_loop_t *loop) : socket(std::make_unique<uv_tcp_t>())
{
  uv_tcp_init(loop, socket.get());
  socket->data = this;
}

auto Tcp::readStart(ReadCb cb) -> int
{
  readCb = std::move(cb);
  socket->data = this;
  return uv_read_start((uv_stream_t *)socket.get(),
                       [](uv_handle_t *handle, size_t suggestedSize, uv_buf_t *aBuf) {
                         auto self = static_cast<Tcp *>(handle->data);
                         self->buf.resize(suggestedSize);
                         aBuf->base = self->buf.data();
                         aBuf->len = suggestedSize;
                       },
                       [](uv_stream_t *stream, ssize_t nread, const uv_buf_t *aBuf) {
                         static_cast<Tcp *>(stream->data)->onRead(nread, aBuf);
                       });
}

namespace
{
  struct WriteCtx
  {
    Tcp *tcp;
    std::string buf;
    uv_buf_t uvBuf;
    Tcp::WriteCb cb;
  };
} // namespace

auto Tcp::write(std::string val, WriteCb cb) -> int
{
  // TODO-Mika Can we combine two memory allocations into one?
  socket->data = this;
  auto req = new uv_write_t;
  auto writeCtx = new WriteCtx;
  req->data = writeCtx;
  writeCtx->tcp = this;
  const auto sz = static_cast<int>(val.size());
  writeCtx->buf = std::move(val);
  writeCtx->uvBuf = uv_buf_init(writeCtx->buf.data(), sz);
  writeCtx->cb = std::move(cb);
  return uv_write(
    req, (uv_stream_t *)socket.get(), &writeCtx->uvBuf, 1, [](uv_write_t *aReq, int status) {
      auto ctx = static_cast<WriteCtx *>(aReq->data);
      ctx->tcp->onWrite(status, std::move(ctx->cb));
      delete ctx;
      delete aReq;
    });
}

auto Tcp::onRead(ssize_t nread, const uv_buf_t *aBuf) -> void
{
  if (nread < 0)
  {
    LOG(__func__, "error:", uv_err_name(static_cast<int>(nread)));
    readCb(static_cast<int>(nread), std::string{});
    return;
  }
  if (!readCb)
    return;
  readCb(0, std::string{aBuf->base, aBuf->base + nread});
}

Uv::Uv() : loop_(uv_default_loop())
{
  loop_->data = this;
}

auto Uv::tick() -> int
{
  return uv_run(loop_, UV_RUN_NOWAIT);
}

namespace
{
  struct ConnectCtx
  {
    Uv *uv;
    Uv::ConnectCb cb;
  };
  struct ConnectCtx2
  {
    Uv *uv;
    Uv::ConnectCb cb;
    Tcp tcp;
  };
} // namespace

auto Uv::connect(const std::string &domain, const std::string &port, ConnectCb cb) -> int
{
  // TODO-Mika Can we combine two memory allocations into one?
  auto resolver = new uv_getaddrinfo_t;
  auto connectCtx = new ConnectCtx;
  resolver->data = connectCtx;
  connectCtx->uv = this;
  connectCtx->cb = std::move(cb);
  LOG("Resolve:", domain, port);
  struct addrinfo hints;
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = 0;
  return uv_getaddrinfo(
    loop_,
    resolver,
    [](uv_getaddrinfo_t *req, int status, struct addrinfo *res) -> void {
      auto ctx = static_cast<ConnectCtx *>(req->data);
      ctx->uv->onResolved(status, res, std::move(ctx->cb));
      delete ctx;
      delete req;
    },
    domain.c_str(),
    port.c_str(),
    &hints);
}

auto Uv::onResolved(int status, struct addrinfo *res, ConnectCb cb) -> void
{
  if (status < 0)
  {
    LOG(__func__, "error:", uv_err_name(status));
    cb(status, Tcp{});
    return;
  }

  char addr[17] = {'\0'};
  uv_ip4_name((struct sockaddr_in *)res->ai_addr, addr, 16);
  LOG("Resolved:", addr);

  // TODO-Mika Can we combine two memory allocations into one?
  auto connectReq = new uv_connect_t;
  auto connectCtx = new ConnectCtx2;
  connectReq->data = connectCtx;
  connectCtx->uv = this;
  connectCtx->tcp = Tcp{loop_};

  auto s = uv_tcp_connect(connectReq,
                          connectCtx->tcp.socket.get(),
                          (const struct sockaddr *)res->ai_addr,
                          [](uv_connect_t *req, int aStatus) {
                            LOG("Connected");
                            auto ctx = static_cast<ConnectCtx2 *>(req->data);
                            ctx->uv->onConnected(aStatus, std::move(ctx->tcp), std::move(ctx->cb));
                            delete ctx;
                            delete req;
                          });
  if (s < 0)
  {
    LOG(__func__, "error:", uv_err_name(s));
    cb(s, Tcp{});
    uv_freeaddrinfo(res);
    return;
  }
  connectCtx->cb = std::move(cb);
  uv_freeaddrinfo(res);
}

auto Uv::onConnected(int status, Tcp tcp, ConnectCb cb) -> void
{
  cb(status, std::move(tcp));
}

auto Uv::getTimer() -> Timer
{
  return Timer{loop_};
}

Tcp::~Tcp()
{
  if (socket)
  {
    LOG("Graceful disconnect");
    auto rawSocket = socket.release();
    uv_read_stop((uv_stream_t *)rawSocket);
    auto req = new uv_shutdown_t;
    req->data = rawSocket;
    uv_shutdown(req, (uv_stream_t *)rawSocket, [](uv_shutdown_t *handle, int status) {
      LOG("Disconnected");
      if (status < 0)
        LOG(__func__, "error:", uv_err_name(status));
      delete static_cast<uv_tcp_t *>(handle->data);
      delete handle;
    });
  }
}

Timer::Timer(uv_loop_t *loop) : timer(std::make_unique<uv_timer_t>())
{
  uv_timer_init(loop, timer.get());
  timer->data = this;
}

auto Timer::start(Cb aCb, uint64_t timeout, uint64_t repeat) -> int
{
  cb = std::move(aCb);
  timer->data = this;
  return uv_timer_start(
    timer.get(),
    [](uv_timer_t *handle) {
      auto self = static_cast<Timer *>(handle->data);
      if (!self->cb)
        return;
      self->cb();
    },
    timeout,
    repeat);
}

auto Timer::stop() -> int
{
  return uv_timer_stop(timer.get());
}

Timer::~Timer()
{
  stop();
}

auto Uv::loop() const -> decltype(uv_default_loop())
{
  return loop_;
}
