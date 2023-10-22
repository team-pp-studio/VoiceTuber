#include "uv.hpp"
#include <spdlog/spdlog.h>
#include <sstream>
#include <stdexcept>

namespace uv
{
  Tcp::Tcp(uv_loop_t *loop)
    : socket(std::make_unique<uv_tcp_t>())
  {
    SPDLOG_DEBUG("{}", this);
    uv_tcp_init(loop, socket.get());
    socket->data = this;
  }

  auto Tcp::readStart(ReadCallback cb) -> int
  {
    SPDLOG_DEBUG("{}: {}", this, socket);
    if (!socket)
    {
      SPDLOG_DEBUG("{}: uninitialized TCP connection", this);
      cb(static_cast<int>(-1), std::string{});
      return -1;
    }
    readCb = std::move(cb);
    socket->data = this;
    return uv_read_start((uv_stream_t *)socket.get(),
                         [](uv_handle_t *handle, size_t suggestedSize, uv_buf_t *aBuf) {
                           auto self = static_cast<Tcp *>(handle->data);
                           if (!self)
                           {
                             SPDLOG_DEBUG("self is nullptr");
                             return;
                           }
                           self->buf.resize(suggestedSize);
                           aBuf->base = self->buf.data();
                           // NOTE: uv_buf_t::len type is system dependant
                           aBuf->len = static_cast<decltype(uv_buf_t::len)>(suggestedSize);
                         },
                         [](uv_stream_t *stream, ssize_t nread, const uv_buf_t *aBuf) {
                           auto self = static_cast<Tcp *>(stream->data);
                           if (!self)
                           {
                             SPDLOG_DEBUG("self is nullptr");
                             return;
                           }
                           self->onRead(nread, aBuf);
                         });
  }

  namespace
  {
    struct WriteCtx
    {
      Tcp *tcp;
      std::string buf;
      uv_buf_t uvBuf;
      Tcp::WriteCallback cb;
    };
  } // namespace

  auto Tcp::write(std::string val, WriteCallback cb) -> int
  {
    if (!socket)
    {
      SPDLOG_DEBUG("{}: uninitialized TCP connection", this);
      cb(-1);
      return -1;
    }

    socket->data = this;
    struct Request : uv_write_t
    {
      WriteCtx ctx;
    };

    auto req = std::make_unique<Request>();
    req->ctx.tcp = this;
    const auto sz = static_cast<int>(val.size());
    req->ctx.buf = std::move(val);
    req->ctx.uvBuf = uv_buf_init(req->ctx.buf.data(), sz);
    req->ctx.cb = std::move(cb);
    return uv_write(
      req.release(), (uv_stream_t *)socket.get(), &req->ctx.uvBuf, 1, [](uv_write_t *aReq, int status) {
        auto req = std::unique_ptr<Request>(static_cast<Request *>(aReq));
        req->ctx.tcp->onWrite(status, std::move(req->ctx.cb));
      });
  }

  auto Tcp::onRead(ssize_t nread, const uv_buf_t *aBuf) -> void
  {
    if (nread < 0)
    {
      SPDLOG_ERROR("{}", uv_err_name(static_cast<int>(nread)));
      readCb(static_cast<int>(nread), std::string_view{});
      return;
    }
    if (!readCb)
    {
      SPDLOG_DEBUG("{}: The TCP read callback is not set up", this);
      return;
    }
    readCb(0, std::string_view{aBuf->base, aBuf->base + nread});
  }

  Uv::Uv()
    : loop_(uv_default_loop())
  {
    loop_->data = this;
  }

  auto Uv::tick() -> int
  {
    return uv_run(loop_, UV_RUN_ONCE);
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
    SPDLOG_INFO("connect {}:{}", domain, port);
    // TODO-Mika Can we combine two memory allocations into one?
    struct Request : uv_getaddrinfo_t
    {
      ConnectCtx ctx;
    };

    auto req = std::make_unique<Request>();

    req->ctx.uv = this;
    req->ctx.cb = std::move(cb);
    SPDLOG_INFO("Resolve: {}:{}", domain, port);
    struct addrinfo hints;
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = 0;
    return uv_getaddrinfo(
      loop_,
      req.release(),
      [](uv_getaddrinfo_t *aReq, int status, struct addrinfo *res) -> void {
        auto req = std::unique_ptr<Request>(static_cast<Request *>(aReq));
        req->ctx.uv->onResolved(status, res, std::move(req->ctx.cb));
      },
      domain.c_str(),
      port.c_str(),
      &hints);
  }

  auto Uv::onResolved(int status, struct addrinfo *res, ConnectCb cb) -> void
  {

    if (status < 0)
    {
      SPDLOG_ERROR("{}", uv_err_name(status));
      cb(status, Tcp{});
      return;
    }
    auto result = std::unique_ptr<addrinfo, void (*)(addrinfo *)>(res, &uv_freeaddrinfo);

    char addr[17] = {'\0'};
    uv_ip4_name(reinterpret_cast<struct sockaddr_in *>(result->ai_addr), addr, 16);
    SPDLOG_INFO("Resolved: {}", addr);

    // TODO-Mika Can we combine two memory allocations into one?
    struct Request : uv_connect_t
    {
      ConnectCtx2 ctx;
    };
    auto req = std::make_unique<Request>();
    req->ctx.uv = this;
    req->ctx.tcp = Tcp{loop_};
    req->ctx.cb = std::move(cb);

    auto sock = req->ctx.tcp.socket.get();
    auto s = uv_tcp_connect(req.release(),
                            sock,
                            (const struct sockaddr *)result->ai_addr,
                            [](uv_connect_t *aReq, int aStatus) {
                              auto req = std::unique_ptr<Request>(static_cast<Request *>(aReq));
                              SPDLOG_INFO("Connected");

                              req->ctx.uv->onConnected(
                                aStatus, std::move(req->ctx.tcp), std::move(req->ctx.cb));
                            });
    if (s < 0)
    {
      SPDLOG_ERROR("{}", uv_err_name(s));
      cb(s, Tcp{});

      return;
    }
  }

  auto Uv::onConnected(int status, Tcp tcp, ConnectCb cb) -> void
  {
    cb(status, std::move(tcp));
  }

  auto Uv::createTimer() -> Timer
  {
    return Timer{loop_};
  }

  auto Uv::createIdle() -> Idle
  {
    return Idle{loop_};
  }

  auto Uv::createPrepare() -> Prepare
  {
    return Prepare{loop_};
  }

  Tcp::Tcp(Tcp &&other)
    : socket(std::move(other.socket)), readCb(std::move(other.readCb)), buf(std::move(other.buf))
  {
    if (socket)
      socket->data = this;
    other.socket = nullptr;
  }

  auto Tcp::deinit() -> void
  {
    if (socket)
    {
      SPDLOG_DEBUG("{}: graceful disconnect", this);
      socket->data = nullptr;
      auto rawSocket = socket.release();
      uv_read_stop((uv_stream_t *)rawSocket);
      struct Request : uv_shutdown_t
      {
        Request(uv_tcp_t *tcp) { this->handle = reinterpret_cast<uv_stream_t *>(tcp); }
        ~Request() { delete static_cast<uv_tcp_t *>(this->data); }
      };
      auto req = std::make_unique<uv_shutdown_t>(rawSocket);
      uv_shutdown(req.release(), (uv_stream_t *)rawSocket, [](uv_shutdown_t *aReq, int status) {
        auto req = std::unique_ptr<Request>(static_cast<Request *>(aReq));
        SPDLOG_DEBUG("Disconnected");
        if (status < 0)
          SPDLOG_ERROR("{}", uv_err_name(status));
      });
    }
  }

  Tcp &Tcp::operator=(Tcp &&other)
  {
    deinit();

    socket = std::move(other.socket);
    readCb = std::move(other.readCb);
    buf = std::move(other.buf);
    if (socket)
      socket->data = this;
    other.socket = nullptr;
    return *this;
  }

  Tcp::~Tcp()
  {
    deinit();
  }

  Timer::Timer(uv_loop_t *loop)
    : timer(std::make_unique<uv_timer_t>())
  {
    uv_timer_init(loop, timer.get());
    timer->data = this;
  }

  auto Timer::start(Callback aCb, uint64_t timeout, uint64_t repeat) -> int
  {
    callback = std::move(aCb);
    timer->data = this;
    return uv_timer_start(
      timer.get(),
      [](uv_timer_t *handle) {
        auto self = static_cast<Timer *>(handle->data);
        if (!self->callback)
        {
          SPDLOG_DEBUG("The Timer callback is not set up");
          return;
        }
        self->callback();
      },
      timeout,
      repeat);
  }

  auto Timer::stop() -> int
  {
    if (!timer)
    {
      SPDLOG_DEBUG("timer is nullptr");
      return -1;
    }
    return uv_timer_stop(timer.get());
  }

  Timer::~Timer()
  {
    stop();
  }

  auto Uv::loop() const -> uv_loop_t *
  {
    return loop_;
  }

  Idle::Idle(uv_loop_t *loop)
    : idle(std::make_unique<uv_idle_t>())
  {
    uv_idle_init(loop, idle.get());
    idle->data = this;
  }

  Idle::~Idle()
  {
    stop();
  }

  auto Idle::start(Cb aCb) -> int
  {
    cb = std::move(aCb);
    idle->data = this;
    return uv_idle_start(idle.get(), [](uv_idle_t *handle) {
      auto self = static_cast<Idle *>(handle->data);
      if (!self->cb)
      {
        SPDLOG_DEBUG("The Idle callback is not set up");
        return;
      }
      self->cb();
    });
  }

  auto Idle::stop() -> int
  {
    if (!idle)
    {
      SPDLOG_DEBUG("idle is nullptr");
      return -1;
    }
    return uv_idle_stop(idle.get());
  }

  Prepare::Prepare(uv_loop_t *loop)
    : prepare(std::make_unique<uv_prepare_t>())
  {
    uv_prepare_init(loop, prepare.get());
    prepare->data = this;
  }

  Prepare::~Prepare()
  {
    stop();
  }

  auto Prepare::start(Cb aCb) -> int
  {
    cb = std::move(aCb);
    prepare->data = this;
    return uv_prepare_start(prepare.get(), [](uv_prepare_t *handle) {
      auto self = static_cast<Prepare *>(handle->data);
      if (!self->cb)
      {
        SPDLOG_DEBUG("The Prepare callback is not set up");
        return;
      }
      self->cb();
    });
  }

  auto Prepare::stop() -> int
  {
    if (!prepare)
    {
      SPDLOG_DEBUG("prepare is nullptr");
      return -1;
    }
    return uv_prepare_stop(prepare.get());
  }

  FsEvent::FsEvent(uv_loop_t *loop)
    : event(std::make_unique<uv_fs_event_t>())
  {
    uv_fs_event_init(loop, event.get());
    event->data = this;
  }

  FsEvent::~FsEvent()
  {
    stop();
  }

  auto FsEvent::start(Cb aCb, const std::string &path, unsigned flags) -> int
  {
    cb = std::move(aCb);
    event->data = this;
    return uv_fs_event_start(
      event.get(),
      [](uv_fs_event_t *handle, const char *filename, int events, int status) {
        auto self = static_cast<FsEvent *>(handle->data);
        if (!self->cb)
        {
          SPDLOG_DEBUG("The FsEvent callback is not set up");
          return;
        }
        self->cb(filename, events, status);
      },
      path.c_str(),
      flags);
  }

  auto FsEvent::stop() -> int
  {
    if (!event)
    {
      SPDLOG_DEBUG("event is nullptr");
      return -1;
    }
    return uv_fs_event_stop(event.get());
  }

  auto Uv::createFsEvent() -> FsEvent
  {
    return FsEvent{loop_};
  }
} // namespace uv
