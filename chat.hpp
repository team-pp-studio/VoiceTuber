#pragma once

#include "lib.hpp"
#include "node.hpp"
#include "twitch-sink.hpp"
#include "twitch.hpp"
#include <memory>

class Chat final : public Node, public TwitchSink
{
public:
#define SER_PROP_LIST
  SER_DEF_PROPS()
#undef SER_PROP_LIST

  static constexpr const char *className = "Chat";

  Chat(class Lib &, std::string name);
  ~Chat() final;

private:
  std::shared_ptr<Twitch> twitch;
  std::shared_ptr<Font> font;
  std::vector<Msg> msgs;

  auto load(IStrm &) -> void final;
  auto onMsg(Msg) -> void final;
  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
  auto save(OStrm &) const -> void final;
  auto h() const -> float final;
  auto w() const -> float final;
};
