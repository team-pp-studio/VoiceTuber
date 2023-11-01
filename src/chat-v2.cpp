#include "chat-v2.hpp"

ChatV2::ChatV2(Lib &lib, Undo &aUndo, uv::Uv &uv, class AudioSink &audioSink, std::string aName)
  : Chat(lib, aUndo, uv, audioSink, aName)
{
}

auto ChatV2::load(IStrm &strm) -> void
{
  ::deser(strm, *this);
  std::string tmpClassName;
  std::string tmpName;
  ::deser(strm, tmpClassName);
  ::deser(strm, tmpName);
  Chat::load(strm);
}

auto ChatV2::save(OStrm &strm) const -> void
{
  ::ser(strm, className);
  ::ser(strm, name);
  ::ser(strm, *this);
  Chat::save(strm);
}

auto ChatV2::do_clone() const -> std::shared_ptr<Node>
{
  return std::make_shared<ChatV2>(*this);
}
