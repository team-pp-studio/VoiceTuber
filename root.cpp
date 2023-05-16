#include "root.hpp"
#include "ui.hpp"
#include <SDL_opengl.h>
#include <limits>
#include <log/log.hpp>

Root::Root(Lib &lib) : Node(lib, "root") {}

Root::~Root() {}

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
  ImGui::ColorEdit4(
    "##BG color", (float *)&clearColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
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
