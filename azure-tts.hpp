#pragma once
#include <functional>
#include <queue>
#include <string>

class AzureTts
{
public:
  AzureTts(std::string key, class HttpClient &, class AudioSink &);
  ~AzureTts();
  auto say(std::string voice, std::string msg) -> void;
  auto updateKey(std::string key) -> void;

private:
  enum class State { idle, waiting };

  std::string key;
  std::reference_wrapper<HttpClient> httpClient;
  std::reference_wrapper<AudioSink> audioSink;
  std::string token;
  std::queue<std::pair<std::string, std::string>> queue;
  State state = State::idle;

  auto getToken() -> void;
  auto process() -> void;
};
