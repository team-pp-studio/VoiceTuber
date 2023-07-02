#pragma once
#include "uv.hpp"
#include <chrono>
#include <deque>
#include <functional>
#include <optional>
#include <queue>
#include <string>

class Gpt
{
public:
  using Cb = std::function<auto(const std::string &)->void>;
  Gpt(uv::Uv &, std::string token, class HttpClient &);
  auto cohost() const -> std::string;
  auto cohost(std::string) -> void;
  auto prompt(std::string name, std::string msg, Cb) -> void;
  auto systemPrompt() const -> const std::string &;
  auto systemPrompt(std::string) -> void;
  auto updateToken(std::string token) -> void;

  std::string lastError;

private:
  struct Msg
  {
    std::string name;
    std::string msg;
  };
  enum class State { idle, waiting };

  std::shared_ptr<int> alive;
  uv::Timer timer;
  std::string token;
  std::string systemPrompt_ =
    R"(Clara is a virtual co-host for Mika's Twitch stream. She entertains
the audience, keeps the energy high, and contributes to the fun
atmosphere. She is funny and likes to make quirky jokes. She has
extensive knowledge about games, game development, Unreal Engine, and
C++.)";
  std::reference_wrapper<HttpClient> httpClient;
  std::vector<std::pair<Msg, Cb>> queuedMsgs;
  std::deque<Msg> msgs;
  State state = State::idle;
  decltype(std::chrono::high_resolution_clock::now()) lastReply;
  std::string cohost_ = "Clara";

  auto countWords() const -> int;
  auto countWords(const Msg &) const -> int;
  auto process() -> void;
};
