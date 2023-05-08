#pragma once
#include <functional>
#include <memory>
#include <string>
#include <uv.h>

class Tcp
{
public:
  using ReadCb = std::function<auto(std::string)->void>;
  using WriteCb = std::function<auto(int status)->void>;
  std::unique_ptr<uv_tcp_t> socket = nullptr;

  Tcp() = default;
  Tcp(uv_loop_t *);
  auto readStart(ReadCb) -> int;
  auto write(std::string, WriteCb) -> int;

private:
  ReadCb readCb;

  auto onRead(ssize_t nread, const uv_buf_t *buf) -> void;
  auto onWrite(int status, WriteCb cb) -> void { cb(status); }
};

class Uv
{
public:
  using ConnectCb = std::function<auto(int status, Tcp)->void>;

  Uv();
  auto connect(const std::string &domain, const std::string &port, ConnectCb) -> int;
  auto tick() -> int;

private:
  decltype(uv_default_loop()) loop;

  auto onConnected(int status, Tcp, ConnectCb) -> void;
  auto onResolved(int status, struct addrinfo *res, ConnectCb) -> void;
};
