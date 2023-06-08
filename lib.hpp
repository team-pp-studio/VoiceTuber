#pragma once
#include "azure-stt.hpp"
#include "azure-token.hpp"
#include "azure-tts.hpp"
#include "font.hpp"
#include "gpt.hpp"
#include "texture.hpp"
#include "twitch.hpp"
#include <map>
#include <memory>
#include <unordered_map>

class Lib
{
public:
  Lib(class Preferences &, uv::Uv &, HttpClient &);
  auto flush() -> void;
  auto queryFont(const std::string &, int size) -> std::shared_ptr<Font>;
  auto queryTex(const std::string &, bool isUi = false) -> std::shared_ptr<const Texture>;
  auto queryTwitch(const std::string &) -> std::shared_ptr<Twitch>;
  auto queryAzureTts(class AudioSink &) -> std::shared_ptr<AzureTts>;
  auto queryAzureStt() -> std::shared_ptr<AzureStt>;
  auto gpt() -> Gpt &;

private:
  std::reference_wrapper<Preferences> preferences;
  std::reference_wrapper<uv::Uv> uv;
  std::reference_wrapper<HttpClient> httpClient;
  std::map<std::pair<std::string, bool>, std::weak_ptr<const Texture>> textures;
  std::unordered_map<std::string, std::weak_ptr<Twitch>> twitchChannels;
  std::map<std::pair<std::string, int>, std::weak_ptr<Font>> fonts;
  AzureToken azureToken;
  std::weak_ptr<AzureTts> azureTts;
  std::weak_ptr<AzureStt> azureStt;
  Gpt gpt_;
};
