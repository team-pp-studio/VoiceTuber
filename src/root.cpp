#include "root.hpp"
#include "ui.hpp"
#include "undo.hpp"
#include <SDL_opengl.h>
#include <limits>
#include <spdlog/spdlog.h>

Root::Root(Lib &lib, Undo &aUndo)
  : Node(lib, aUndo, "root") {}

Root::~Root() {}

auto Root::do_clone() const -> std::shared_ptr<Node>
{
  return std::make_shared<Root>(*this);
}

auto Root::render(float dt, Node *hovered, Node *selected) -> void
{
  zOrder = INT_MIN;
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
  glClear(GL_COLOR_BUFFER_BIT);
  Node::render(dt, hovered, selected);
}

auto Root::renderUi() -> void
{
  Node::renderUi();
  ImGui::TableNextColumn();
  Ui::textRj("BG color");
  ImGui::TableNextColumn();
  auto oldColor = clearColor;
  if (ImGui::ColorEdit4(
        "##BG color", (float *)&clearColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
  {
    undo.get().record(
      [alive = weak_self(), newColor = clearColor]() {
        if (auto self = alive.lock())
        {
          static_cast<Root *>(self.get())->clearColor = std::move(newColor);
        }
        else
        {
          SPDLOG_INFO("this was destroyed");
        }
      },
      [alive = weak_self(), oldColor]() {
        if (auto self = alive.lock())
        {
          static_cast<Root *>(self.get())->clearColor = std::move(oldColor);
        }
        else
        {
          SPDLOG_INFO("this was destroyed");
        }
      },
      "##BG color");
  }
}

auto Root::save(OStrm &strm) const -> void
{
  ::ser(strm, className);
  ::ser(strm, name);
  ::ser(strm, *this);
  Node::save(strm);
}
auto Root::load(IStrm &strm) -> void
{
  ::deser(strm, *this);
  Node::load(strm);
}
