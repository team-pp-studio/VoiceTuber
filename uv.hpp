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
    Tcp(Tcp &&) = default;
    Tcp(uv_loop_t *);
    Tcp &operator=(Tcp &&) = default;
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
  };

  class Timer
  {
  public:
    using Cb = std::function<auto()->void>;

    Timer(uv_loop_t *);
    ~Timer();
    auto start(Cb, uint64_t timeout = 0, uint64_t repeat = 0) -> int;
    auto stop() -> int;

  private:
    std::unique_ptr<uv_timer_t> timer;
    Cb cb = nullptr;
  };

  class Idle
  {
  public:
    using Cb = std::function<auto()->void>;
    Idle(uv_loop_t *);
    ~Idle();
    auto start(Cb) -> int;
    auto stop() -> int;

  private:
    std::unique_ptr<uv_idle_t> idle;
    Cb cb = nullptr;
  };

  class Prepare
  {
  public:
    using Cb = std::function<auto()->void>;
    Prepare(uv_loop_t *);
    ~Prepare();
    auto start(Cb) -> int;
    auto stop() -> int;

  private:
    std::unique_ptr<uv_prepare_t> prepare;
    Cb cb = nullptr;
  };

  class Uv
  {
  public:
    using ConnectCb = std::function<auto(int status, Tcp)->void>;

    Uv();
    auto connect(const std::string &domain, const std::string &port, ConnectCb) -> int;
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
