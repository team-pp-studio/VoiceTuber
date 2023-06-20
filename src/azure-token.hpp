#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

class AzureToken
{
public:
  using Cb = std::function<auto(const std::string &token, const std::string &err)->void>;
  AzureToken(std::string key, class HttpClient &);
  auto clear() -> void;
  auto get(Cb) -> void;
  auto updateKey(const std::string &) -> void;

private:
  std::shared_ptr<int> alive;
  std::string key;
  std::reference_wrapper<HttpClient> httpClient;
  std::vector<Cb> cbs;
  std::string token;
};
