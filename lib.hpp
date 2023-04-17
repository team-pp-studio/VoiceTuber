#pragma once
#include "font.hpp"
#include "texture.hpp"
#include "twitch.hpp"
#include <map>
#include <memory>
#include <unordered_map>

class Lib
{
public:
  auto queryTex(const std::string &) -> std::shared_ptr<const Texture>;
  auto queryTwitch(const std::string &) -> std::shared_ptr<Twitch>;
  auto queryFont(const std::string &, int size) -> std::shared_ptr<Font>;
  auto tick(float dt) -> void;

private:
  std::unordered_map<std::string, std::weak_ptr<const Texture>> textures;
  std::unordered_map<std::string, std::weak_ptr<Twitch>> twitchChannels;
  std::map<std::pair<std::string, int>, std::weak_ptr<Font>> fonts;
};
