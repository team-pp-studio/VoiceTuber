#pragma once
#include "uv.hpp"
#include <functional>
#include <queue>
#include <string>

namespace uv
{
  class Uv;
}

class AzureTts
{
public:
  using ListVoicesCb = std::function<auto(std::vector<std::string>)->void>;
  AzureTts(uv::Uv &, std::string key, class HttpClient &, class AudioSink &);
  auto say(std::string voice, std::string msg) -> void;
  auto updateKey(std::string key) -> void;
  auto listVoices(ListVoicesCb) -> void;

  std::string lastError;

private:
  enum class State { idle, waiting };
  using PostTask = std::function<auto(bool)->void>;
  using Task = std::function<auto(PostTask)->void>;

  uv::Timer timer;
  std::string key;
  std::reference_wrapper<HttpClient> httpClient;
  std::reference_wrapper<AudioSink> audioSink;
  std::string token;
  std::queue<Task> queue;
  State state = State::idle;

  auto getToken() -> void;
  auto process() -> void;
};
