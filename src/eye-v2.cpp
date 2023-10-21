#include "eye-v2.hpp"
#include "mouse-tracking.hpp"
#include "ui.hpp"
#include "undo.hpp"
#include <log/log.hpp>
#include <numbers>

EyeV2::EyeV2(MouseTracking &mouseTracking, Lib &lib, Undo &aUndo, const std::filesystem::path &path)
  : AnimSprite(lib, aUndo, path), mouseTracking(mouseTracking)
{
  mouseTracking.reg(*this);

  auto displayName = SDL_GetDisplayName(0);
  selectedDisplay = displayName;
  SDL_Rect rect;
  SDL_GetDisplayBounds(0, &rect);
  screenTopLeft = glm::vec2{rect.x, rect.y};
  screenBottomRight = glm::vec2{rect.x + rect.w, rect.y + rect.h};
}

EyeV2::~EyeV2()
{
  mouseTracking.get().unreg(*this);
}

auto EyeV2::render(float dt, Node *hovered, Node *selected) -> void
{
  auto clampMouse = [&]() {
    const auto mousePivot = (mouse - pivot()) * followStrength / 100.f;
    const auto distance = glm::length(mousePivot);

    if (distance > radius)
    {
      auto normalizedDirection = glm::normalize(mousePivot);
      return normalizedDirection * radius;
    }
    return mousePivot;
  }();

  glTranslatef(clampMouse.x, clampMouse.y, .0f);
  AnimSprite::render(dt, hovered, selected);
  if (selected == this)
  {
    glBegin(GL_LINE_LOOP);
    const auto NumSegments = 100;
    for (auto i = 0; i < NumSegments; ++i)
    {
      const auto theta = 2.0f * std::numbers::pi_v<float> * i / NumSegments;
      const auto dx = radius * cosf(theta);
      const auto dy = radius * sinf(theta);
      glVertex2f(pivot().x + dx, pivot().y + dy);
    }
    glEnd();
  }
}

auto EyeV2::save(OStrm &strm) const -> void
{
  ::ser(strm, className);
  ::ser(strm, name);
  ::ser(strm, *this);
  ::ser(strm, static_cast<const AnimSprite &>(*this));
  sprite.save(strm);
  Node::save(strm);
}

auto EyeV2::load(IStrm &strm) -> void
{
  ::deser(strm, *this);
  ::deser(strm, static_cast<AnimSprite &>(*this));
  sprite.load(strm);
  Node::load(strm);
}

auto EyeV2::renderUi() -> void
{
  AnimSprite::renderUi();
  ImGui::TableNextColumn();
  Ui::textRj("Radius");
  ImGui::TableNextColumn();
  Ui::dragFloat(undo,
                "##Radius",
                radius,
                1,
                1,
                std::numeric_limits<float>::max(),
                "%.1f",
                ImGuiSliderFlags_AlwaysClamp);
  ImGui::TableNextColumn();
  Ui::textRj("Strength");
  ImGui::TableNextColumn();
  Ui::dragFloat(undo,
                "##Strength",
                followStrength,
                1,
                1,
                std::numeric_limits<float>::max(),
                "%.1f",
                ImGuiSliderFlags_AlwaysClamp);
  ImGui::TableNextColumn();
  Ui::textRj("Display");
  ImGui::TableNextColumn();
  {
    auto combo = Ui::Combo("##Display", selectedDisplay.c_str(), 0);
    if (combo)
    {
      if (ImGui::Selectable("Custom", "Custom" == selectedDisplay))
        undo.get().record(
          [alive = this->weak_from_this()]() {
            if (auto self = std::static_pointer_cast<EyeV2>(alive.lock()))
            {
              self->selectedDisplay = "Custom";
            }
            else
            {
              LOG("this was destroyed");
            }
          },
          [alive = this->weak_from_this(), oldDisplay = selectedDisplay]() {
            if (auto self = std::static_pointer_cast<EyeV2>(alive.lock()))
            {
              self->selectedDisplay = std::move(oldDisplay);
            }
            else
            {
              LOG("this was destroyed");
            }
          });
      const auto displayCnt = SDL_GetNumVideoDisplays();
      for (auto i = 0; i < displayCnt; ++i)
      {
        auto displayName = std::string(SDL_GetDisplayName(i));
        if (ImGui::Selectable(displayName.c_str(), displayName == selectedDisplay))
        {
          undo.get().record(
            [alive = this->weak_from_this(), newDisplay = displayName, i]() {
              if (auto self = std::static_pointer_cast<EyeV2>(alive.lock()))
              {
                self->selectedDisplay = std::move(newDisplay);
                SDL_Rect rect;
                SDL_GetDisplayBounds(i, &rect);
                self->screenTopLeft = glm::vec2{rect.x, rect.y};
                self->screenBottomRight = glm::vec2{rect.x + rect.w, rect.y + rect.h};
              }
              else
              {
                LOG("this was destroyed");
              }
            },
            [alive = this->weak_from_this(),
             oldDisplay = selectedDisplay,
             oldScreenTopLeft = screenTopLeft,
             oldScreenBottomRight = screenBottomRight]() {
              if (auto self = std::static_pointer_cast<EyeV2>(alive.lock()))
              {
                self->selectedDisplay = std::move(oldDisplay);
                self->screenTopLeft = std::move(oldScreenTopLeft);
                self->screenBottomRight = std::move(oldScreenBottomRight);
              }
              else
              {
                LOG("this was destroyed");
              }
            });
        }
      }
    }
  }
  {
    auto disabled = Ui::Disabled{selectedDisplay != "Custom"};
    ImGui::TableNextColumn();
    Ui::textRj("Top Left");
    ImGui::TableNextColumn();
    Ui::inputInt(undo, "X##TopLeft", screenTopLeft.x);
    Ui::inputInt(undo, "Y##TopLeft", screenTopLeft.y);
    ImGui::TableNextColumn();
    Ui::textRj("Bottom Right");
    ImGui::TableNextColumn();
    Ui::inputInt(undo, "X##BottomRight", screenBottomRight.x);
    Ui::inputInt(undo, "Y##BottomRight", screenBottomRight.y);
  }
}

auto EyeV2::ingest(const glm::mat4 &projMat, glm::vec2 v) -> void
{
  v.x -= screenTopLeft.x;
  v.y -= screenTopLeft.y;
  auto &io = ImGui::GetIO();
  v.x = v.x * io.DisplaySize.x / (screenBottomRight.x - screenTopLeft.x);
  v.y = v.y * io.DisplaySize.y / (screenBottomRight.y - screenTopLeft.y);
  mouse = screenToLocal(projMat, v);
}
