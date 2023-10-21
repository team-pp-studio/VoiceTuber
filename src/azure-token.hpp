#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

class AzureToken : public std::enable_shared_from_this<AzureToken>
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
