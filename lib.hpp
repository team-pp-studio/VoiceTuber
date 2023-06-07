#pragma once
#include "azure-tts.hpp"
#include "font.hpp"
#include "texture.hpp"
#include "twitch.hpp"
#include <map>
#include <memory>
#include <unordered_map>

class Lib
{
public:
  Lib(class Preferences &);
  auto flush() -> void;
  auto queryFont(const std::string &, int size) -> std::shared_ptr<Font>;
  auto queryTex(const std::string &, bool isUi = false) -> std::shared_ptr<const Texture>;
  auto queryTwitch(uv::Uv &, const std::string &) -> std::shared_ptr<Twitch>;
  auto queryAzureTts(uv::Uv &, class HttpClient &, class AudioSink &) -> std::shared_ptr<AzureTts>;

private:
  std::reference_wrapper<Preferences> preferences;
  std::map<std::pair<std::string, bool>, std::weak_ptr<const Texture>> textures;
  std::unordered_map<std::string, std::weak_ptr<Twitch>> twitchChannels;
  std::map<std::pair<std::string, int>, std::weak_ptr<Font>> fonts;
  std::weak_ptr<AzureTts> azureTts;
};
