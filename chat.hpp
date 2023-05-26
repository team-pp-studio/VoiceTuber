#pragma once

#include "lib.hpp"
#include "node.hpp"
#include "twitch-sink.hpp"
#include "twitch.hpp"
#include <memory>

class Chat final : public Node, public TwitchSink
{
public:
#define SER_PROP_LIST \
  SER_PROP(ptsize);   \
  SER_PROP(size);     \
  SER_PROP(tts);
  SER_DEF_PROPS()
#undef SER_PROP_LIST

  static constexpr const char *className = "Chat";

  Chat(Lib &, Undo &, class Uv &, class HttpClient &, class AudioSink &, std::string name);
  ~Chat() final;

private:
  int ptsize = 40;
  glm::vec2 size = {400.f, 200.f};
  std::reference_wrapper<Lib> lib;
  std::reference_wrapper<HttpClient> httpClient;
  std::reference_wrapper<AudioSink> audioSink;
  std::shared_ptr<Twitch> twitch;
  std::shared_ptr<Font> font;
  std::vector<Msg> msgs;
  Timer timer;
  bool showChat = false;
  bool tts = false;
  std::shared_ptr<AzureTts> azureTts;

  auto h() const -> float final;
  auto load(IStrm &) -> void final;
  auto onMsg(Msg) -> void final;
  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
  auto save(OStrm &) const -> void final;
  auto w() const -> float final;
  auto wrapText(const std::string &text, float initOffset) const -> std::vector<std::string>;
};
