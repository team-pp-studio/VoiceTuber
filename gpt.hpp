#pragma once
#include "uv.hpp"
#include <deque>
#include <functional>
#include <queue>
#include <string>

class Gpt
{
public:
  using Cb = std::function<auto(const std::string &)->void>;
  Gpt(uv::Uv &, std::string token, std::string systemPrompt, class HttpClient &);
  auto prompt(std::string name, std::string msg, Cb) -> void;
  auto queueSize() const -> int;
  auto updateToken(std::string token) -> void;

  std::string lastError;

private:
  struct Msg
  {
    std::string name;
    std::string msg;
  };
  enum class State { idle, waiting };
  using PostTask = std::function<auto(bool)->void>;
  using Task = std::function<auto(PostTask)->void>;

  uv::Timer timer;
  std::string token;
  std::string systemPrompt;
  std::reference_wrapper<HttpClient> httpClient;
  std::queue<Task> queue;
  std::deque<Msg> msgs;
  State state = State::idle;

  auto countWords() const -> int;
  auto countWords(const Msg &) const -> int;
  auto process() -> void;
};
