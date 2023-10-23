#pragma once
#include "uv.hpp"
#include "wav.hpp"
#include <functional>
#include <queue>
#include <string>

class AzureStt : public std::enable_shared_from_this<AzureStt>
{
public:
  using Callback = std::move_only_function<void(std::string_view)>;
  AzureStt(uv::Uv &, class AzureToken &, class HttpClient &);
  auto perform(Wav, int sampleRate, Callback) -> void;

  std::string lastError;

private:
  enum class State { idle,
                     waiting };
  using PostTask = std::move_only_function<void(bool)>;
  using Task = std::move_only_function<void(const std::string &t, PostTask)>;

  uv::Timer timer;
  std::reference_wrapper<AzureToken> token;
  std::reference_wrapper<HttpClient> httpClient;
  std::queue<Task> queue;
  State state = State::idle;
  float total = 0.f;

  auto process() -> void;
};
