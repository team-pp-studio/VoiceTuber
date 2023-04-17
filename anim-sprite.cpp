#include "anim-sprite.hpp"
#include <imgui/imgui.h>
#include <log/log.hpp>

AnimSprite::AnimSprite(Lib &lib, const std::string &fileName)
  : Sprite(lib, fileName), startTime(std::chrono::high_resolution_clock::now())
{
}

static auto getProjectionMatrix() -> glm::mat4
{
  GLfloat projectionMatrixData[16];
  glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrixData);
  return glm::make_mat4(projectionMatrixData);
}

auto AnimSprite::render(float dt, Node *hovered, Node *selected) -> void
{
  frame = static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(
                             std::chrono::high_resolution_clock::now() - startTime)
                             .count() *
                           fps / 1'000'000) %
          numFrames;
  Sprite::render(dt, hovered, selected);
  if (dt <= 0.f)
    return;

  const auto projMat = getProjectionMatrix();
  const auto pivot4 = glm::vec4{pivot.x, pivot.y, 0.f, 1.f};
  const auto projPivot = projMat * modelViewMat * pivot4;
  const auto v = (glm::vec2{projPivot.x, projPivot.y} - lastProjPivot) / dt;
  const auto gForce = glm::vec2{0.f, .01f};
  const auto a = (v - lastProjPivotV + gForce) / dt;
  lastProjPivot = glm::vec2{projPivot.x, projPivot.y};
  lastProjPivotV = v;

  if (!physics)
    return;

  if (glm::length(end - pivot) < 1.f)
    return;

  const auto end4 = glm::vec4{end.x, end.y, 0.f, 1.f};
  const auto projEnd = projMat * modelViewMat * end4;
  const auto pivotToEnd = glm::vec2(projEnd) - glm::vec2(projPivot);
  const auto orthogonalVec = glm::vec2{-pivotToEnd.y, pivotToEnd.x};
  const auto normalizedOrthogonalVec = glm::normalize(orthogonalVec);
  float projection = glm::dot(a, normalizedOrthogonalVec);
  animRotV += (-force * projection - animRot * springness - animRotV * damping) * dt;
  animRot += animRotV * dt;

  if (selected != this)
    return;
  glColor4f(1.f, .7f, .0f, 1.f);
  glBegin(GL_LINES);
  glVertex3f(pivot.x, pivot.y, zOrder / 1024.f);
  glVertex3f(end.x, end.y, zOrder / 1024.f);
  glEnd();
}

auto AnimSprite::renderUi() -> void
{
  Sprite::renderUi();
  ImGui::PushID("AnimSprite");
  ImGui::PushItemWidth(ImGui::GetFontSize() * 16 + 8);
  ImGui::DragFloat(
    "FPS", &fps, 1, 1, std::numeric_limits<float>::max(), "%.1f", ImGuiSliderFlags_AlwaysClamp);
  ImGui::Checkbox("Physics", &physics);

  ImGui::BeginDisabled(!physics);
  ImGui::PushID("End");
  ImGui::PushItemWidth(ImGui::GetFontSize() * 8);
  ImGui::DragFloat("##XEnd",
                   &end.x,
                   1.f,
                   -std::numeric_limits<float>::max(),
                   std::numeric_limits<float>::max(),
                   "%.1f");
  ImGui::SameLine();
  ImGui::DragFloat(
    "End", &end.y, 1.f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), "%.1f");
  if (ImGui::Button("NW"))
  {
    end = glm::vec2{0, h()};
  }
  ImGui::SameLine();
  if (ImGui::Button("N "))
  {
    end = glm::vec2{w() / 2, h()};
  }
  ImGui::SameLine();
  if (ImGui::Button("NE"))
  {
    end = glm::vec2{w(), h()};
  }
  if (ImGui::Button("W "))
  {
    end = glm::vec2{w(), h() / 2};
  }
  ImGui::SameLine();
  if (ImGui::Button("C "))
  {
    end = glm::vec2{w() / 2, h() / 2};
  }
  ImGui::SameLine();
  if (ImGui::Button("E "))
  {
    end = glm::vec2{w(), h() / 2};
  }
  if (ImGui::Button("SW"))
  {
    end = glm::vec2{0, 0};
  }
  ImGui::SameLine();
  if (ImGui::Button("S "))
  {
    end = glm::vec2{w() / 2, 0};
  }
  ImGui::SameLine();
  if (ImGui::Button("SE"))
  {
    end = glm::vec2{w(), 0};
  }
  ImGui::PopItemWidth();
  ImGui::PopID();
  ImGui::DragFloat(
    "Force", &force, .1f, 0, std::numeric_limits<float>::max(), "%.1f", ImGuiSliderFlags_AlwaysClamp);
  ImGui::DragFloat("Damping",
                   &damping,
                   .1f,
                   0,
                   std::numeric_limits<float>::max(),
                   "%.1f",
                   ImGuiSliderFlags_AlwaysClamp);
  ImGui::DragFloat("Springness",
                   &springness,
                   .1f,
                   0,
                   std::numeric_limits<float>::max(),
                   "%.1f",
                   ImGuiSliderFlags_AlwaysClamp);
  ImGui::EndDisabled();
  ImGui::PopItemWidth();
  ImGui::PopID();
}

auto AnimSprite::save(OStrm &strm) const -> void
{
  ::ser(strm, className);
  ::ser(strm, name);
  ::ser(strm, *this);
  Sprite::save(strm);
}

auto AnimSprite::load(IStrm &strm) -> void
{
  ::deser(strm, *this);
  Sprite::load(strm);
}
