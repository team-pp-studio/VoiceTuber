#include "sprite.hpp"
#include "stb/stb_image.h"
#include <cassert>
#include <cstdint>
#include <imgui/imgui.h>

Sprite::Sprite(const std::string &fileName)
  : texture([&]() {
      int ch;
      stbi_set_flip_vertically_on_load(1);
      auto imageData = stbi_load(fileName.c_str(), &w_, &h_, &ch, STBI_rgb_alpha);
      if (imageData == nullptr)
      {
        std::ostringstream ss;
        ss << "Error loading image: " << fileName;
        throw std::runtime_error(ss.str());
      }
      assert(ch == 4 && "Since we asked for rgb_alpha format, the number of channels should be 4.");

      GLuint ret;
      glGenTextures(1, &ret);
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w_, h_, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
      stbi_image_free(imageData);
      return texture;
    }())
{
}

auto Sprite::render(Node *hovered, Node *selected) -> void
{
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture);
  glBegin(GL_QUADS);
  const auto fCols = static_cast<float>(cols);
  const auto fRows = static_cast<float>(rows);
  auto i = frame % cols / fCols;
  auto j = (fRows - 1.f - frame / cols) / fRows;
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
  Node::render(hovered, selected);
}

Sprite::~Sprite()
{
  glDeleteTextures(1, &texture);
}

auto Sprite::renderUi() -> void
{
  Node::renderUi();
  ImGui::PushID("Sprite");
  ImGui::PushItemWidth(ImGui::GetFontSize() * 16 + 8);
  ImGui::DragInt(
    "Cols", &cols, 1, 1, std::numeric_limits<int>::max(), "%d", ImGuiSliderFlags_AlwaysClamp);
  ImGui::DragInt(
    "Rows", &rows, 1, 1, std::numeric_limits<int>::max(), "%d", ImGuiSliderFlags_AlwaysClamp);
  ImGui::PopItemWidth();
  ImGui::PopID();
}
