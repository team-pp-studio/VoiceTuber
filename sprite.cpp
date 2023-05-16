#include "sprite.hpp"
#include "ui.hpp"
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <imgui/imgui.h>
#include <log/log.hpp>

Sprite::Sprite(Lib &lib, const std::filesystem::path &path)
  : Node(lib, [&path]() { return path.filename().string(); }()), texture(lib.queryTex([&path]() {
      try
      {
        if (!std::filesystem::exists(path.filename()))
          std::filesystem::copy(path, path.filename());
        return path.filename().string();
      }
      catch (std::runtime_error &e)
      {
        LOG(e.what());
        return std::string{"engine:missing.png"};
      }
    }()))
{
}

auto Sprite::render(float dt, Node *hovered, Node *selected) -> void
{
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture->texture());
  glBegin(GL_QUADS);
  const auto fCols = static_cast<float>(cols);
  const auto fRows = static_cast<float>(rows);
  const auto i = frame % cols / fCols;
  const auto j = (fRows - 1.f - frame / cols) / fRows;
  glColor4f(1.f, 1.f, 1.f, 1.f);
  glTexCoord2f(.0f + i, .0f + j);
  glVertex2f(.0f, .0f);
  glTexCoord2f(1.f / fCols + i, .0f + j);
  glVertex2f(w(), .0f);
  glTexCoord2f(1.f / fCols + i, 1.f / fRows + j);
  glVertex2f(w(), h());
  glTexCoord2f(.0f + i, 1.f / fRows + j);
  glVertex2f(.0f, h());
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);
  Node::render(dt, hovered, selected);
}

auto Sprite::renderUi() -> void
{
  Node::renderUi();
  ImGui::TableNextColumn();
  Ui::textRj("Cols");
  ImGui::TableNextColumn();
  ImGui::InputInt("##Cols", &cols);
  if (cols < 1)
    cols = 1;
  ImGui::TableNextColumn();
  Ui::textRj("Rows");
  ImGui::TableNextColumn();
  ImGui::InputInt("##Rows", &rows);
  if (rows < 1)
    rows = 1;
  ImGui::TableNextColumn();
  Ui::textRj("NumFrames");
  ImGui::TableNextColumn();
  ImGui::InputInt("##NumFrames", &numFrames);
  if (numFrames < 1)
    numFrames = 1;
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
  return 1.f * texture->w() / cols;
}

auto Sprite::h() const -> float
{
  return 1.f * texture->h() / rows;
}

auto Sprite::isTransparent(glm::vec2 v) const -> bool
{
  if (texture->ch() == 3)
    return false;
  const auto x = static_cast<int>(v.x + (frame % numFrames) % cols * w());
  const auto y = static_cast<int>(v.y + (rows - (frame % numFrames) / cols - 1) * h());
  if (x < 0 || x >= texture->w() || y < 0 || y >= texture->h())
    return true;

  if (auto imageData = texture->imageData())
    return imageData[(x + y * texture->w()) * texture->ch() + 3] < 127;
  else
    return false;
}
