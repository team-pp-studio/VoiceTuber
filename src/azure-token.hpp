#pragma once
#include <functional>
#include <string>
#include <vector>

#include "shared_from_this.hpp"

class AzureToken : public virtual enable_shared_from_this
{
public:
  using Callback = std::move_only_function<auto(const std::string &token, const std::string &err)->void>;
  AzureToken(std::string key, class HttpClient &);
  auto clear() -> void;
  auto get(Callback) -> void;
  auto updateKey(const std::string &) -> void;

private:
  std::string key;
  std::reference_wrapper<HttpClient> httpClient;
  std::vector<Callback> callbacks;
  std::string token;
};
