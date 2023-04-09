#include "sprite.hpp"
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <imgui/imgui.h>
#include <log/log.hpp>

Sprite::Sprite(TexLib &texLib, const std::string &path)
  : Node([&path]() {
      std::filesystem::path fsPath(path);
      return fsPath.filename().string();
    }()),
    texture(texLib.queryTex(path))
{
}

auto Sprite::render(float dt, Node *hovered, Node *selected) -> void
{
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture.get().texture());
  glBegin(GL_QUADS);
  const auto fCols = static_cast<float>(cols);
  const auto fRows = static_cast<float>(rows);
  const auto i = frame % cols / fCols;
  const auto j = (fRows - 1.f - frame / cols) / fRows;
  glColor4f(1.f, 1.f, 1.f, 1.f);
  glTexCoord2f(.0f + i, .0f + j);
  glVertex3f(.0f, .0f, zOrder / 1024.f);
  glTexCoord2f(1.f / fCols + i, .0f + j);
  glVertex3f(w(), .0f, zOrder / 1024.f);
  glTexCoord2f(1.f / fCols + i, 1.f / fRows + j);
  glVertex3f(w(), h(), zOrder / 1024.f);
  glTexCoord2f(.0f + i, 1.f / fRows + j);
  glVertex3f(.0f, h(), zOrder / 1024.f);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);
  Node::render(dt, hovered, selected);
}

auto Sprite::renderUi() -> void
{
  Node::renderUi();
  ImGui::PushID("Sprite");
  ImGui::PushItemWidth(ImGui::GetFontSize() * 16 + 8);
  ImGui::InputInt("Cols", &cols);
  if (cols < 1)
    cols = 1;
  ImGui::InputInt("Rows", &rows);
  if (rows < 1)
    rows = 1;
  ImGui::InputInt("NumFrames", &numFrames);
  if (numFrames < 1)
    numFrames = 1;
  ImGui::PopItemWidth();
  ImGui::PopID();
}

auto Sprite::save(OStrm &strm) const -> void
{
  ::ser(strm, *this);
  Node::save(strm);
}

auto Sprite::load(IStrm &strm) -> void
{
  ::deser(strm, *this);
  Node::load(strm);
}

auto Sprite::w() const -> float
{
  return 1.f * texture.get().w() / cols;
}

auto Sprite::h() const -> float
{
  return 1.f * texture.get().h() / rows;
}

auto Sprite::isTransparent(glm::vec2 v) const -> bool
{
  if (texture.get().ch() == 3)
    return false;
  const auto x = static_cast<int>(v.x + (frame % numFrames) % cols * w());
  const auto y = static_cast<int>(v.y + (rows - (frame % numFrames) / cols - 1) * h());
  if (x < 0 || x >= texture.get().w() || y < 0 || y >= texture.get().h())
    return true;
  return texture.get().imageData()[(x + y * texture.get().w()) * texture.get().ch() + 3] < 127;
}
