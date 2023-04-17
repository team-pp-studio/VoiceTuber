#include "lib.hpp"
#include <cassert>
#include <fstream>

auto Lib::queryTex(const std::string &v) -> std::shared_ptr<const Texture>
{
  auto it = textures.find(v);
  if (it != std::end(textures))
  {
    auto shared = it->second.lock();
    if (shared)
      return shared;
    textures.erase(it);
  }
  auto shared = std::make_shared<Texture>(v);
  auto tmp = textures.emplace(v, shared);
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
  auto shared = std::make_shared<Twitch>(
    []() {
      auto f = std::ifstream("/home/mika/prj/twitch_tts/twitch_auth.txt"); // TODO-Mika remove hardcoded values
      auto key = std::string{};
      std::getline(f, key);
      return key;
    }(),
    "mika314",
    v);
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

auto Lib::tick(float dt) -> void
{
  for (auto &twitch : twitchChannels)
  {
    auto l = twitch.second.lock();
    if (!l)
      continue;
    l->tick(dt);
  }
}
