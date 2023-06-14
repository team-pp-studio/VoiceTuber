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
  AzureTts(uv::Uv &, class AzureToken &, class HttpClient &, class AudioSink &);
  auto say(std::string voice, std::string msg, bool overlap = true) -> void;
  auto listVoices(ListVoicesCb) -> void;

  std::string lastError;

private:
  enum class State { idle, waiting };
  using PostTask = std::function<auto(bool)->void>;
  using Task = std::function<auto(const std::string &t, PostTask)->void>;

  std::shared_ptr<int> alive;
  uv::Timer timer;
  std::reference_wrapper<AzureToken> token;
  std::reference_wrapper<HttpClient> httpClient;
  std::reference_wrapper<AudioSink> audioSink;
  std::queue<Task> queue;
  State state = State::idle;

  auto process() -> void;
};
