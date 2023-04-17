#pragma once
#include "twitch-sink.hpp"
#include <SDL_net.h>
#include <deque>
#include <string>
#include <vector>

class Twitch
{
public:
  Twitch(std::string key, std::string user, std::string channel);
  Twitch(const Twitch &) = delete;
  ~Twitch();
  auto tick(float dt) -> void;
  auto reg(TwitchSink &) -> void;
  auto unreg(TwitchSink &) -> void;

private:
  std::string key;
  std::string user;
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
