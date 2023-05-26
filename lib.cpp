#include "lib.hpp"
#include "preferences.hpp"
#include <cassert>
#include <log/log.hpp>

Lib::Lib(class Preferences &preferences) : preferences(preferences) {}

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
  auto shared = std::make_shared<Texture>(v, isUi);
  auto tmp = textures.emplace(std::pair{v, isUi}, shared);
  assert(tmp.second);
  return shared;
}

auto Lib::queryTwitch(class Uv &uv, const std::string &v) -> std::shared_ptr<Twitch>
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
  auto tmp = twitchChannels.emplace(v, shared);
  assert(tmp.second);
  return shared;
}

auto Lib::queryFont(const std::string &path, int size) -> std::shared_ptr<Font>
{
  auto it = fonts.find({path, size});
  if (it != std::end(fonts))
  {
    auto shared = it->second.lock();
    if (shared)
      return shared;
    fonts.erase(it);
  }
  auto shared = std::make_shared<Font>(path, size);
  auto tmp = fonts.emplace(std::pair{path, size}, shared);
  assert(tmp.second);
  return shared;
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
  if (auto aTts = azureTts.lock())
    aTts->updateKey(preferences.get().azureKey);
}

auto Lib::queryAzureTts(class HttpClient &httpClient, class AudioSink &audioSink)
  -> std::shared_ptr<AzureTts>
{
  if (auto ret = azureTts.lock())
    return ret;
  auto ret = std::make_shared<AzureTts>(preferences.get().azureKey, httpClient, audioSink);
  azureTts = ret;
  return ret;
}
