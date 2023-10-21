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
    using ReadCallback = std::move_only_function<void(int status, std::string_view)>;
    using WriteCallback = std::move_only_function<void(int status)>;

    Tcp() = default;
    Tcp(Tcp &&);
    Tcp(uv_loop_t *);
    Tcp &operator=(Tcp &&);
    ~Tcp();
    auto readStart(ReadCallback) -> int;
    auto write(std::string, WriteCallback) -> int;
    auto isInitialized() const -> bool { return socket != nullptr; }

  private:
    std::unique_ptr<uv_tcp_t> socket = nullptr;
    ReadCallback readCb;
    std::string buf;

    auto onRead(ssize_t nread, const uv_buf_t *buf) -> void;
    auto onWrite(int status, WriteCallback cb) -> void { cb(status); }
    auto deinit() -> void;
  };

  class Timer
  {
    friend class Uv;

  public:
    using Callback = std::move_only_function<void()>;

    ~Timer();
    auto start(Callback, uint64_t timeout = 0, uint64_t repeat = 0) -> int;
    auto stop() -> int;

  private:
    Timer(uv_loop_t *);
    std::unique_ptr<uv_timer_t> timer;
    Callback callback = nullptr;
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
    auto loop() const -> uv_loop_t *;
    auto tick() -> int;

  private:
    uv_loop_t *loop_;

    auto onConnected(int status, Tcp, ConnectCb) -> void;
    auto onResolved(int status, struct addrinfo *res, ConnectCb) -> void;
  };
} // namespace uv
