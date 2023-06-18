#pragma once
#include "uv.hpp"
#include "wav.hpp"
#include <functional>
#include <queue>
#include <string>

class AzureStt
{
public:
  using Cb = std::function<auto(std::string)->void>;
  AzureStt(uv::Uv &, class AzureToken &, class HttpClient &);
  auto perform(Wav, int sampleRate, Cb) -> void;

  std::string lastError;

private:
  enum class State { idle, waiting };
  using PostTask = std::function<auto(bool)->void>;
  using Task = std::function<auto(const std::string &t, PostTask)->void>;

  std::shared_ptr<int> alive;
  uv::Timer timer;
  std::reference_wrapper<AzureToken> token;
  std::reference_wrapper<HttpClient> httpClient;
  std::queue<Task> queue;
  State state = State::idle;

  auto process() -> void;
};
