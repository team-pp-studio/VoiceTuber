#pragma once
#include "twitch-sink.hpp"
#include "uv.hpp"
#include <deque>
#include <functional>
#include <string>
#include <vector>

namespace uv
{
  class Uv;
}

class Twitch
{
public:
  Twitch(uv::Uv &, std::string user, std::string key, std::string channel);
  Twitch(const Twitch &) = delete;
  auto isConnected() const -> bool;
  auto reg(TwitchSink &) -> void;
  auto unreg(TwitchSink &) -> void;
  auto updateUserKey(const std::string &user, const std::string &key) -> void;

private:
  std::shared_ptr<int> alive;
  std::reference_wrapper<uv::Uv> uv;
  std::string user;
  std::string key;
  std::string channel;
  uv::Tcp tcp;
  enum class State { connecting, connected };
  State state = State::connecting;
  std::deque<char> buf;
  std::vector<std::reference_wrapper<TwitchSink>> sinks;
  int initRetry = 1000;
  uv::Timer retryTimer;
  uv::Timer pingTimer;

  auto init() -> void;
  auto initiateRetry() -> void;
  auto sendPassNickUser() -> void;
  auto readStart() -> void;
  auto parseMsg() -> void;
  auto onWelcome() -> void;
  auto onPing(const std::string &) -> void;
  auto onPong() -> void;
  auto schedulePing() -> void;
};
