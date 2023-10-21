#pragma once
#include "uv.hpp"
#include <functional>
#include <queue>
#include <string>

namespace uv
{
  class Uv;
}

class AzureTts : std::enable_shared_from_this<AzureTts>
{
public:
  using ListVoicesCallback = std::move_only_function<void(std::vector<std::string>)>;
  AzureTts(uv::Uv &, class AzureToken &, class HttpClient &, class AudioSink &);
  auto say(std::string voice, std::string msg, bool overlap = true) -> void;
  auto listVoices(ListVoicesCallback) -> void;

  std::string lastError;

private:
  enum class State { idle, waiting };
  using PostTask = std::move_only_function<void(bool)>;
  using Task = std::move_only_function<void(const std::string &t, PostTask)>;

  uv::Timer timer;
  std::reference_wrapper<AzureToken> token;
  std::reference_wrapper<HttpClient> httpClient;
  std::reference_wrapper<AudioSink> audioSink;
  std::queue<Task> queue;
  State state = State::idle;

  auto process() -> void;
};
