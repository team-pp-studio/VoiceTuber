#pragma once

#include "chat.hpp"

class ChatV2 final : public Chat
{
public:
#define SER_PROP_LIST SER_PROP(hideChatSec);

  SER_DEF_PROPS()
#undef SER_PROP_LIST

  ChatV2(Lib &, Undo &, uv::Uv &, class AudioSink &, std::string name);

  static constexpr const char *className = "Chat-v2";

private:
  auto load(IStrm &) -> void final;
  auto save(OStrm &) const -> void final;
  auto do_clone() const -> std::shared_ptr<Node> final;
};
