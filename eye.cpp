#include "eye.hpp"
#include "mouse-tracking.hpp"
#include "ui.hpp"
#include <numbers>

Eye::Eye(MouseTracking &mouseTracking, Lib &lib, Undo &undo, const std::filesystem::path &path)
  : AnimSprite(lib, undo, path), mouseTracking(mouseTracking)
{
  mouseTracking.reg(*this);
}

Eye::~Eye()
{
  mouseTracking.get().unreg(*this);
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
      const auto theta = 2.0f * std::numbers::pi_v<float> * i / NumSegments;
      const auto dx = radius * cosf(theta);
      const auto dy = radius * sinf(theta);
      glVertex2f(pivot.x + dx, pivot.y + dy);
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
  ImGui::TableNextColumn();
  Ui::textRj("Radius");
  ImGui::TableNextColumn();
  ImGui::DragFloat(
    "##Radius", &radius, 1, 1, std::numeric_limits<float>::max(), "%.1f", ImGuiSliderFlags_AlwaysClamp);
  ImGui::TableNextColumn();
  Ui::textRj("Strength");
  ImGui::TableNextColumn();
  ImGui::DragFloat("##Strength",
                   &followStrength,
                   1,
                   1,
                   std::numeric_limits<float>::max(),
                   "%.1f",
                   ImGuiSliderFlags_AlwaysClamp);
}

auto Eye::ingest(const glm::mat4 &projMat, glm::vec2 v) -> void
{
  mouse = screenToLocal(projMat, v);
}
