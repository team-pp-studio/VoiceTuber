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
  return uv_read_start((uv_stream_t *)socket.get(),
                       [](uv_handle_t * /*handle*/, size_t suggestedSize, uv_buf_t *buf) {
                         // TODO-Mika Can we avoid extra memory allocation?
                         buf->base = new char[suggestedSize];
                         buf->len = suggestedSize;
                       },
                       [](uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
                         static_cast<Tcp *>(stream->data)->onRead(nread, buf);
                         delete buf->base;
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
  auto req = new uv_write_t;
  auto writeCtx = new WriteCtx;
  req->data = writeCtx;
  writeCtx->tcp = this;
  writeCtx->buf = std::move(val);
  writeCtx->uvBuf = uv_buf_init(writeCtx->buf.data(), static_cast<int>(val.size()));
  writeCtx->cb = std::move(cb);
  return uv_write(
    req, (uv_stream_t *)socket.get(), &writeCtx->uvBuf, 1, [](uv_write_t *aReq, int status) {
      auto ctx = static_cast<WriteCtx *>(aReq->data);
      ctx->tcp->onWrite(status, std::move(ctx->cb));
      delete ctx;
      delete aReq;
    });
}

auto Tcp::onRead(ssize_t nread, const uv_buf_t *buf) -> void
{
  if (!readCb)
    return;
  readCb(std::string{buf->base, buf->base + nread});
}

Uv::Uv() : loop(uv_default_loop())
{
  loop->data = this;
}

auto Uv::tick() -> int
{
  return uv_run(loop, UV_RUN_NOWAIT);
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
    loop,
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
    LOG("getaddrinfo callback error", uv_err_name(status));
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
  connectCtx->cb = std::move(cb);

  uv_tcp_connect(connectReq,
                 connectCtx->tcp.socket.get(),
                 (const struct sockaddr *)res->ai_addr,
                 [](uv_connect_t *req, int aStatus) {
                   auto ctx = static_cast<ConnectCtx2 *>(req->data);
                   ctx->uv->onConnected(aStatus, std::move(ctx->tcp), std::move(ctx->cb));
                   delete ctx;
                   delete req;
                 });
  uv_freeaddrinfo(res);
}

auto Uv::onConnected(int status, Tcp tcp, ConnectCb cb) -> void
{
  cb(status, std::move(tcp));
}
