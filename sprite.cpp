#include "sprite.hpp"
#include "stb/stb_image.h"
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <imgui/imgui.h>
#include <log/log.hpp>

Sprite::Sprite(const std::string &path)
  : Node([&path]() {
      std::filesystem::path fsPath(path);
      return fsPath.filename().string();
    }()),
    texture([&]() {
      stbi_set_flip_vertically_on_load(1);
      imageData = stbi_load(name.c_str(), &w_, &h_, &ch, STBI_rgb_alpha);
      if (imageData == nullptr)
      {
        std::ostringstream ss;
        ss << "Error loading image: " << name;
        throw std::runtime_error(ss.str());
      }
      LOG("Number of channels:", ch);
      assert((ch == 4 || ch == 3) && "The number of channels should be 3 or 4.");

      GLuint ret;
      glGenTextures(1, &ret);
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      if (ch == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w_, h_, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
      else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w_, h_, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
      return texture;
    }())
{
}

auto Sprite::render(float dt, Node *hovered, Node *selected) -> void
{
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture);
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

Sprite::~Sprite()
{
  glDeleteTextures(1, &texture);
  stbi_image_free(imageData);
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
  return 1.f * w_ / cols;
}

auto Sprite::h() const -> float
{
  return 1.f * h_ / rows;
}

auto Sprite::isTransparent(glm::vec2 v) const -> bool
{
  if (ch == 3)
    return false;
  const auto x = static_cast<int>(v.x + (frame % numFrames) % cols * w());
  const auto y = static_cast<int>(v.y + (rows - (frame % numFrames) / cols - 1) * w());
  if (x < 0 || x >= w_ || y < 0 || y >= h_)
    return true;
  return imageData[(x + y * w_) * ch + 3] < 127;
}
