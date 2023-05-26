#pragma once
#include "uv.hpp"
#include <functional>
#include <queue>
#include <string>

class AzureTts
{
public:
  using ListVoicesCb = std::function<auto(std::vector<std::string>)->void>;
  AzureTts(class Uv &, std::string key, class HttpClient &, class AudioSink &);
  auto say(std::string voice, std::string msg) -> void;
  auto updateKey(std::string key) -> void;
  auto listVoices(ListVoicesCb) -> void;

private:
  enum class State { idle, waiting };
  using PostTask = std::function<auto(bool)->void>;
  using Task = std::function<auto(PostTask)->void>;

  Timer timer;
  std::string key;
  std::reference_wrapper<HttpClient> httpClient;
  std::reference_wrapper<AudioSink> audioSink;
  std::string token;
  std::queue<Task> queue;
  State state = State::idle;

  auto getToken() -> void;
  auto process() -> void;
};
