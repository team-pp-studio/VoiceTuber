#pragma once
#include "twitch-sink.hpp"
#include <SDL_net.h>
#include <deque>
#include <functional>
#include <string>
#include <vector>

class Twitch
{
public:
  Twitch(class Uv &, std::string user, std::string key, std::string channel);
  Twitch(const Twitch &) = delete;
  ~Twitch();
  auto isConnected() const -> bool;
  auto reg(TwitchSink &) -> void;
  auto tick(float dt) -> void;
  auto unreg(TwitchSink &) -> void;
  auto updateUserKey(const std::string &user, const std::string &key) -> void;

private:
  std::reference_wrapper<Uv> uv;
  std::string user;
  std::string key;
  std::string channel;
  TCPsocket socket = nullptr;
  SDLNet_SocketSet socketSet = nullptr;
  enum class State { connecting, connected };
  State state = State::connecting;
  std::deque<char> buf;
  std::vector<std::reference_wrapper<TwitchSink>> sinks;
  float retry = 0.f;
  float initRetry = 1.f;

  auto init() -> void;
  auto initiateRetry() -> void;
};
