#pragma once
#include <SDL_net.h>
#include <deque>
#include <string>

class Twitch
{
public:
  Twitch(std::string key, std::string user, std::string channel);
  Twitch(const Twitch &) = delete;
  ~Twitch();
  auto tick() -> void;

private:
  std::string key;
  std::string user;
  std::string channel;
  TCPsocket socket = nullptr;
  SDLNet_SocketSet socketSet = nullptr;
  enum class State { connecting, connected };
  State state = State::connecting;
  std::deque<char> buf;
};
