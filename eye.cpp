#include "eye.hpp"
#include "app.hpp"
#include <numbers>

Eye::Eye(App &app, TexLib &texLib, const std::string &fileName) : AnimSprite(texLib, fileName), app(app)
{
  app.reg(*this);
}

Eye::~Eye()
{
  app.get().unreg(*this);
}

auto Eye::render(float dt, Node *hovered, Node *selected) -> void
{
  auto clampMouse = [&]() {
    const auto mousePivot = (mouse - pivot) * followStrength / 100.f;
    const auto distance = glm::length(mousePivot);

    if (distance > radius)
    {
      auto normalizedDirection = glm::normalize(mousePivot);
      return normalizedDirection * radius;
    }
    return mousePivot;
  }();

  glTranslatef(clampMouse.x, clampMouse.y, .0f);
  Sprite::render(dt, hovered, selected);
  if (selected == this)
  {
    glBegin(GL_LINE_LOOP);
    const auto NumSegments = 100;
    for (auto i = 0; i < NumSegments; ++i)
    {
      const auto theta = 2.0f * std::numbers::pi * i / NumSegments;
      const auto dx = radius * cosf(theta);
      const auto dy = radius * sinf(theta);
      glVertex3f(pivot.x + dx, pivot.y + dy, zOrder / 1024.f);
    }
    glEnd();
  }
}

auto Eye::save(OStrm &strm) const -> void
{
  ::ser(strm, className);
  ::ser(strm, name);
  ::ser(strm, *this);
  ::ser(strm, static_cast<const AnimSprite &>(*this));
  Sprite::save(strm);
}

auto Eye::load(IStrm &strm) -> void
{
  ::deser(strm, *this);
  ::deser(strm, static_cast<AnimSprite &>(*this));
  Sprite::load(strm);
}

auto Eye::renderUi() -> void
{
  Sprite::renderUi();
  ImGui::PushID("AnimSprite");
  ImGui::PushItemWidth(ImGui::GetFontSize() * 16 + 8);
  ImGui::DragFloat(
    "Radius", &radius, 1, 1, std::numeric_limits<float>::max(), "%.1f", ImGuiSliderFlags_AlwaysClamp);
  ImGui::DragFloat("Strength",
                   &followStrength,
                   1,
                   1,
                   std::numeric_limits<float>::max(),
                   "%.1f",
                   ImGuiSliderFlags_AlwaysClamp);
  ImGui::PopItemWidth();
  ImGui::PopID();
}

auto Eye::ingest(const glm::mat4 &projMat, glm::vec2 v) -> void
{
  mouse = screenToLocal(projMat, v);
}
