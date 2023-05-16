#include "anim-sprite.hpp"
#include "ui.hpp"
#include <imgui/imgui.h>
#include <log/log.hpp>

AnimSprite::AnimSprite(Lib &lib, const std::filesystem::path &path)
  : Sprite(lib, path),
    startTime(std::chrono::high_resolution_clock::now()),
    arrowN(lib.queryTex("engine:arrow-n-circle.png", true)),
    arrowNE(lib.queryTex("engine:arrow-ne-circle.png", true)),
    arrowE(lib.queryTex("engine:arrow-e-circle.png", true)),
    arrowSE(lib.queryTex("engine:arrow-se-circle.png", true)),
    arrowS(lib.queryTex("engine:arrow-s-circle.png", true)),
    arrowSW(lib.queryTex("engine:arrow-sw-circle.png", true)),
    arrowW(lib.queryTex("engine:arrow-w-circle.png", true)),
    arrowNW(lib.queryTex("engine:arrow-nw-circle.png", true)),
    center(lib.queryTex("engine:center-circle.png", true))
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
  {
    animRotV = {};
    animRot = {};
    return;
  }

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
  glVertex2f(pivot.x, pivot.y);
  glVertex2f(end.x, end.y);
  glEnd();
}

auto AnimSprite::renderUi() -> void
{
  Sprite::renderUi();

  ImGui::TableNextColumn();
  Ui::textRj("FPS");
  ImGui::TableNextColumn();
  ImGui::DragFloat(
    "##FPS", &fps, 1, 1, std::numeric_limits<float>::max(), "%.1f", ImGuiSliderFlags_AlwaysClamp);
  ImGui::TableNextColumn();
  Ui::textRj("Physics");
  ImGui::TableNextColumn();
  ImGui::Checkbox("##Physics", &physics);
  ImGui::TableNextColumn();
  {
    auto physicsDisabled = Ui::Disabled{!physics};
    Ui::textRj("End");
    ImGui::TableNextColumn();
    ImGui::DragFloat("##XEnd",
                     &end.x,
                     1.f,
                     -std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::max(),
                     "%.1f");
    ImGui::DragFloat("##YEnd",
                     &end.y,
                     1.f,
                     -std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::max(),
                     "%.1f");
    const auto sz = 2 * ImGui::GetFontSize();
    if (Ui::BtnImg("nw2", *arrowNW, sz, sz))
    {
      end = glm::vec2{0, h()};
    }
    ImGui::SameLine();
    if (Ui::BtnImg("n2", *arrowN, sz, sz))
    {
      end = glm::vec2{w() / 2, h()};
    }
    ImGui::SameLine();
    if (Ui::BtnImg("ne2", *arrowNE, sz, sz))
    {
      end = glm::vec2{w(), h()};
    }
    if (Ui::BtnImg("w2", *arrowW, sz, sz))
    {
      end = glm::vec2{0, h() / 2};
    }
    ImGui::SameLine();
    if (Ui::BtnImg("c2", *center, sz, sz))
    {
      end = glm::vec2{w() / 2, h() / 2};
    }
    ImGui::SameLine();
    if (Ui::BtnImg("e2", *arrowE, sz, sz))
    {
      end = glm::vec2{w(), h() / 2};
    }
    if (Ui::BtnImg("sw2", *arrowSW, sz, sz))
    {
      end = glm::vec2{0, 0};
    }
    ImGui::SameLine();
    if (Ui::BtnImg("s2", *arrowS, sz, sz))
    {
      end = glm::vec2{w() / 2, 0};
    }
    ImGui::SameLine();
    if (Ui::BtnImg("se2", *arrowSE, sz, sz))
    {
      end = glm::vec2{w(), 0};
    }
    ImGui::TableNextColumn();
    Ui::textRj("Force");
    ImGui::TableNextColumn();
    ImGui::DragFloat("##Force",
                     &force,
                     .1f,
                     0,
                     std::numeric_limits<float>::max(),
                     "%.1f",
                     ImGuiSliderFlags_AlwaysClamp);
    ImGui::TableNextColumn();
    Ui::textRj("Damping");
    ImGui::TableNextColumn();
    ImGui::DragFloat("##Damping",
                     &damping,
                     .1f,
                     0,
                     std::numeric_limits<float>::max(),
                     "%.1f",
                     ImGuiSliderFlags_AlwaysClamp);
    ImGui::TableNextColumn();
    Ui::textRj("Springness");
    ImGui::TableNextColumn();
    ImGui::DragFloat("##Springness",
                     &springness,
                     .1f,
                     0,
                     std::numeric_limits<float>::max(),
                     "%.1f",
                     ImGuiSliderFlags_AlwaysClamp);
  }
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
