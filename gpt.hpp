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
  auto prompt(std::string name, std::string msg, Cb) -> void;
  auto updateToken(std::string token) -> void;
  auto systemPrompt(std::string) -> void;
  auto systemPrompt() const -> const std::string &;

  std::string lastError;

private:
  struct Msg
  {
    std::string name;
    std::string msg;
  };
  enum class State { idle, waiting };

  uv::Timer timer;
  std::string token;
  std::string systemPrompt_ =
    R"(Clara is an AI co-host for Mika's Twitch stream. She entertains the audience, keeps the energy
high, and contributes to the fun atmosphere. She is funny and likes to make quirky jokes. Here is the
transcript of the stream:)";
  std::reference_wrapper<HttpClient> httpClient;
  std::vector<std::pair<Msg, Cb>> queuedMsgs;
  std::deque<Msg> msgs;
  State state = State::idle;
  decltype(std::chrono::high_resolution_clock::now()) lastReply;

  auto countWords() const -> int;
  auto countWords(const Msg &) const -> int;
  auto process() -> void;
};
