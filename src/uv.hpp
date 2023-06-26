#pragma once
#include <functional>
#include <memory>
#include <string>
#include <uv.h>

namespace uv
{
  class Tcp
  {
    friend class Uv;

  public:
    using ReadCb = std::function<auto(int status, std::string)->void>;
    using WriteCb = std::function<auto(int status)->void>;

    Tcp() = default;
    Tcp(Tcp &&);
    Tcp(uv_loop_t *);
    Tcp &operator=(Tcp &&);
    ~Tcp();
    auto readStart(ReadCb) -> int;
    auto write(std::string, WriteCb) -> int;
    auto isInitialized() const -> bool { return socket != nullptr; }

  private:
    std::unique_ptr<uv_tcp_t> socket = nullptr;
    ReadCb readCb;
    std::string buf;

    auto onRead(ssize_t nread, const uv_buf_t *buf) -> void;
    auto onWrite(int status, WriteCb cb) -> void { cb(status); }
    auto deinit() -> void;
  };

  class Timer
  {
    friend class Uv;

  public:
    using Cb = std::function<auto()->void>;

    ~Timer();
    auto start(Cb, uint64_t timeout = 0, uint64_t repeat = 0) -> int;
    auto stop() -> int;

  private:
    Timer(uv_loop_t *);
    std::unique_ptr<uv_timer_t> timer;
    Cb cb = nullptr;
  };

  class Idle
  {
    friend class Uv;

  public:
    using Cb = std::function<auto()->void>;
    ~Idle();
    auto start(Cb) -> int;
    auto stop() -> int;

  private:
    Idle(uv_loop_t *);

    std::unique_ptr<uv_idle_t> idle;
    Cb cb = nullptr;
  };

  class FsEvent
  {
    friend class Uv;

  public:
    using Cb = std::function<auto(std::string filename, int events, int status)->void>;

    FsEvent(FsEvent &&) = default;
    ~FsEvent();
    auto start(Cb, const std::string &path, unsigned flags) -> int;
    auto stop() -> int;

  private:
    FsEvent(uv_loop_t *);
    std::unique_ptr<uv_fs_event_t> event;
    Cb cb = nullptr;
  };

  class Prepare
  {
    friend class Uv;

  public:
    using Cb = std::function<auto()->void>;
    ~Prepare();
    auto start(Cb) -> int;
    auto stop() -> int;

  private:
    Prepare(uv_loop_t *);
    std::unique_ptr<uv_prepare_t> prepare;
    Cb cb = nullptr;
  };

  class Uv
  {
  public:
    using ConnectCb = std::function<auto(int status, Tcp)->void>;

    Uv();
    auto connect(const std::string &domain, const std::string &port, ConnectCb) -> int;
    auto createFsEvent() -> FsEvent;
    auto createIdle() -> Idle;
    auto createPrepare() -> Prepare;
    auto createTimer() -> Timer;
    auto loop() const -> decltype(uv_default_loop());
    auto tick() -> int;

  private:
    decltype(uv_default_loop()) loop_;

    auto onConnected(int status, Tcp, ConnectCb) -> void;
    auto onResolved(int status, struct addrinfo *res, ConnectCb) -> void;
  };
} // namespace uv
