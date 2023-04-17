#include "chat.hpp"
#include "lib.hpp"
#include <sstream>

Chat::Chat(class Lib &lib, std::string n)
  : Node(n),
    twitch(lib.queryTwitch(n)),
    font(lib.queryFont("/home/mika/prj/VoiceTuber/notepad_font/NotepadFont.ttf", 14)) // TODO-Mika do not use hardcoded paths
{
  twitch->reg(*this);
}

Chat::~Chat()
{
  twitch->unreg(*this);
}

auto Chat::onMsg(Msg val) -> void
{
  msgs.emplace_back(std::move(val));
}

auto Chat::save(OStrm &strm) const -> void
{
  ::ser(strm, className);
  ::ser(strm, name);
  ::ser(strm, *this);
  Node::save(strm);
}
auto Chat::load(IStrm &strm) -> void
{
  ::deser(strm, *this);
  Node::load(strm);
}

auto Chat::render(float dt, Node *hovered, Node *selected) -> void
{
  auto y = 0;
  for (auto it = msgs.rbegin(); it != msgs.rend(); ++it)
  {
    auto msg = std::ostringstream{};
    msg << it->displayName << ": " << it->msg;
    font->render(glm::vec3{0.f, y, zOrder / 1024.f}, msg.str());
    y += font->getH(msg.str());
  }

  Node::render(dt, hovered, selected);
}

auto Chat::renderUi() -> void
{
  Node::renderUi();
  if (ImGui::BeginListBox("##Chat", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
  {
    for (const auto &msg : msgs)
      ImGui::Text("%s: %s", msg.displayName.c_str(), msg.msg.c_str());
    ImGui::EndListBox();
  }
}

auto Chat::h() const -> float
{
  return 200.f;
}

auto Chat::w() const -> float
{
  return 400.f;
}
