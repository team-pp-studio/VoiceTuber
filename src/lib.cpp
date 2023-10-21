#include "lib.hpp"
#include "preferences.hpp"
#include <cassert>
#include <log/log.hpp>

Lib::Lib(class Preferences &aPreferences, uv::Uv &aUv, HttpClient &aHttpClient)
  : preferences(aPreferences),
    uv(aUv),
    httpClient(aHttpClient),
    azureToken(preferences.get().azureKey, httpClient),
    gpt_(uv, preferences.get().openAiToken, httpClient)
{
}

auto Lib::queryTex(const std::string &v, bool isUi) -> std::shared_ptr<const Texture>
{
  auto it = textures.find(std::pair{v, isUi});
  if (it != std::end(textures))
  {
    auto shared = it->second.lock();
    if (shared)
      return shared;
    textures.erase(it);
  }
  auto shared = std::make_shared<Texture>(uv, v, isUi);
  [[maybe_unused]] auto tmp = textures.emplace(std::pair{v, isUi}, shared);
  assert(tmp.second);
  return shared;
}

auto Lib::queryTwitch(const std::string &v) -> std::shared_ptr<Twitch>
{
  auto it = twitchChannels.find(v);
  if (it != std::end(twitchChannels))
  {
    auto shared = it->second.lock();
    if (shared)
      return shared;
    twitchChannels.erase(it);
  }
  auto shared =
    std::make_shared<Twitch>(uv, preferences.get().twitchUser, preferences.get().twitchKey, v);
  [[maybe_unused]] auto tmp = twitchChannels.emplace(v, shared);
  assert(tmp.second);
  return shared;
}

auto Lib::queryFont(const std::filesystem::path &path, int size) -> std::shared_ptr<Font>
{
  auto it = fonts.find(std::make_pair(std::cref(path), size));
  if (it != std::end(fonts))
  {
    if (auto shared = it->second.lock())
      return shared;
  }

  auto font = std::make_shared<Font>(path, size);
  fonts.emplace_hint(
    it, std::piecewise_construct, std::forward_as_tuple(path, size), std::forward_as_tuple(font));

  return font;
}

auto Lib::flush() -> void
{
  for (auto &twitch : twitchChannels)
  {
    auto shared = twitch.second.lock();
    if (!shared)
      continue;
    shared->updateUserKey(preferences.get().twitchUser, preferences.get().twitchKey);
  }
  azureToken.updateKey(preferences.get().azureKey);
  gpt_.updateToken(preferences.get().openAiToken);
}

auto Lib::queryAzureTts(class AudioSink &audioSink) -> std::shared_ptr<AzureTts>
{
  if (auto ret = azureTts.lock())
    return ret;
  auto ret = std::make_shared<AzureTts>(uv, azureToken, httpClient, audioSink);
  azureTts = ret;
  return ret;
}

auto Lib::queryAzureStt() -> std::shared_ptr<AzureStt>
{
  if (auto ret = azureStt.lock())
    return ret;
  auto ret = std::make_shared<AzureStt>(uv, azureToken, httpClient);
  azureStt = ret;
  return ret;
}

auto Lib::gpt() -> Gpt &
{
  return gpt_;
}
