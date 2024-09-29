#pragma once
#include <functional>
#include <queue>
#include <span>
#include <string>
#include <string_view>

#include "shared_from_this.hpp"
#include "uv.hpp"

namespace uv
{
  class Uv;
}

class AzureTts : public virtual enable_shared_from_this
{
public:
  using ListVoicesCallback = std::move_only_function<void(std::span<std::string_view>)>;
  AzureTts(uv::Uv &, class AzureToken &, class HttpClient &, class AudioSink &);
  auto say(std::string voice, std::string msg, bool overlap = true) -> void;
  auto listVoices(ListVoicesCallback) -> void;

  std::string lastError;

private:
  enum class State {
    idle,
    waiting,
  };

  using PostTask = std::move_only_function<void(bool)>;
  using Task = std::move_only_function<void(std::string_view, PostTask)>;

  uv::Timer timer;
  std::reference_wrapper<AzureToken> token;
  std::reference_wrapper<HttpClient> httpClient;
  std::reference_wrapper<AudioSink> audioSink;
  std::queue<Task> queue;
  State state = State::idle;

  auto process() -> void;
};
