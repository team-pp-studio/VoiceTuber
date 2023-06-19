#include "preferences.hpp"
#include <SDL.h>
#include <sdlpp/sdlpp.hpp>
#include <log/log.hpp>
#include <filesystem>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcovered-switch-default"
#include <cpptoml.h>
#pragma GCC diagnostic pop

static auto CompanyName = "TeamPP";
static auto AppName = "VoiceTuber";

Preferences::Preferences()
{
  auto prefPath = SDL_GetPrefPath(CompanyName, AppName);
  if (!prefPath)
  {
    LOG("Error getting preferences directory:", SDL_GetError());
    return;
  }

  auto configFilePath = std::string{prefPath};
  configFilePath.append("preferences.toml");
  SDL_free(prefPath);

  if (!std::filesystem::exists(configFilePath)) {
    // create the file
    auto *rwops = SDL_RWFromFile(configFilePath.c_str(), "b");
    if (rwops == nullptr)
      throw sdl::Error(SDL_GetError());
    SDL_RWclose(rwops);
  }

  try
  {
    auto config = cpptoml::parse_file(configFilePath);
    twitchUser = config->get_qualified_as<std::string>("twitch.user").value_or("mika314");
    twitchKey = config->get_qualified_as<std::string>("twitch.key").value_or("");
    audioOut = config->get_qualified_as<std::string>("audio.out").value_or("Default");
    audioIn = config->get_qualified_as<std::string>("audio.in").value_or("Default");
    azureKey = config->get_qualified_as<std::string>("azure.key").value_or("");
    openAiToken = config->get_qualified_as<std::string>("open-ai.token").value_or("");
    vsync = config->get_qualified_as<bool>("graphics.vsync").value_or(true);
    fps = config->get_qualified_as<int>("graphics.fps").value_or(0);
  }
  catch (const cpptoml::parse_exception &e)
  {
    LOG("Error parsing config file:", e.what());
  }
}

auto Preferences::save() -> void
{
  auto prefPath = SDL_GetPrefPath(CompanyName, AppName);
  if (!prefPath)
  {
    std::cerr << "Error getting preferences directory: " << SDL_GetError() << std::endl;
    return;
  }

  auto configFilePath = std::string{prefPath};
  configFilePath.append("preferences.toml");
  SDL_free(prefPath);

  try
  {
    auto config = cpptoml::make_table();

    {
      auto twitchTable = cpptoml::make_table();
      twitchTable->insert("user", twitchUser);
      twitchTable->insert("key", twitchKey);
      config->insert("twitch", twitchTable);
    }
    {
      auto audioTable = cpptoml::make_table();
      audioTable->insert("out", audioOut);
      audioTable->insert("in", audioIn);
      config->insert("audio", audioTable);
    }
    {
      auto azureTable = cpptoml::make_table();
      azureTable->insert("key", azureKey);
      config->insert("azure", azureTable);
    }
    {
      auto openAiTable = cpptoml::make_table();
      openAiTable->insert("token", openAiToken);
      config->insert("open-ai", openAiTable);
    }
    {
      auto graphicsTable = cpptoml::make_table();
      graphicsTable->insert("vsync", vsync);
      graphicsTable->insert("fps", fps);
      config->insert("graphics", graphicsTable);
    }

    auto configFile = std::ofstream{configFilePath};
    if (!configFile.is_open())
    {
      LOG("Error opening config file for writing");
      return;
    }

    auto writer = cpptoml::toml_writer{configFile};
    config->accept(writer);
    configFile.close();
  }
  catch (const std::ofstream::failure &e)
  {
    LOG("Error saving config file:", e.what());
  }
}
