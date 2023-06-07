#pragma once

#include "lib.hpp"
#include "node.hpp"
#include "twitch-sink.hpp"
#include "twitch.hpp"
#include "uv.hpp"
#include <memory>

class Chat : public Node, public TwitchSink
{
public:
#define SER_PROP_LIST \
  SER_PROP(ptsize);   \
  SER_PROP(size);     \
  SER_PROP(tts);      \
  SER_PROP(voicesMap);
  SER_DEF_PROPS()
#undef SER_PROP_LIST

  static constexpr const char *className = "Chat";

  Chat(Lib &, Undo &, uv::Uv &, class HttpClient &, class AudioSink &, std::string name);
  ~Chat() override;

private:
  int ptsize = 40;
  glm::vec2 size = {400.f, 200.f};
  std::reference_wrapper<Lib> lib;
  std::reference_wrapper<uv::Uv> uv;
  std::reference_wrapper<HttpClient> httpClient;
  std::reference_wrapper<AudioSink> audioSink;
  std::shared_ptr<Twitch> twitch;
  std::shared_ptr<Font> font;
  std::vector<Msg> msgs;
  uv::Timer timer;
  bool showChat = false;
  bool tts = false;
  std::shared_ptr<AzureTts> azureTts;
  std::vector<std::string> voices;
  std::string lastName;
  std::unordered_map<std::string, std::string> voicesMap;
  std::string chatterName;
  std::string chatterVoice;

protected:
  int hideChatSec = 0;
  auto load(IStrm &) -> void override;
  auto save(OStrm &) const -> void override;

private:
  auto h() const -> float final;
  auto onMsg(Msg) -> void final;
  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
  auto w() const -> float final;
  auto wrapText(const std::string &text, float initOffset) const -> std::vector<std::string>;
  auto getVoice(const std::string &name) const -> std::string;
};
