#include "node.hpp"
#include <SDL_opengl.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <limits>

static auto getModelViewMatrix() -> glm::mat4
{
  GLfloat modelMatrixData[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, modelMatrixData);
  return glm::make_mat4(modelMatrixData);
}

auto Node::renderAll() -> void
{
  glPushMatrix();
  // Apply transformations
  glTranslatef(loc.x, loc.y, 0.0f);       // Move the sprite
  glTranslatef(pivot.x, pivot.y, 0.0f);   // Move the pivot point
  glRotatef(rot, 0.0f, 0.0f, 1.0f);       // Rotate the sprite
  glScalef(scale.x, scale.y, 1.0f);       // Scale the sprite
  glTranslatef(-pivot.x, -pivot.y, 0.0f); // Move the pivot point back
  modelViewMat = getModelViewMatrix();

  render();
  for (auto n : nodes)
    n.get().renderAll();
  glPopMatrix();
}

auto Node::renderUi() -> void
{
  ImGui::PushID("Location");
  ImGui::PushItemWidth(ImGui::GetFontSize() * 8);
  ImGui::DragFloat("##XLocation",
                   &loc.x,
                   1.f,
                   -std::numeric_limits<float>::max(),
                   std::numeric_limits<float>::max(),
                   "%.1f");
  ImGui::SameLine();
  ImGui::DragFloat("Location",
                   &loc.y,
                   1.f,
                   -std::numeric_limits<float>::max(),
                   std::numeric_limits<float>::max(),
                   "%.1f");
  ImGui::PopItemWidth();
  ImGui::PopID();

  ImGui::PushID("Scale");
  if (uniformScaling)
  {
    ImGui::PushItemWidth(ImGui::GetFontSize() * 16 + 8);
    float avgScale = (scale.x + scale.y) / 2.0f;
    if (ImGui::DragFloat("##X",
                         &avgScale,
                         0.01f,
                         -std::numeric_limits<float>::max(),
                         std::numeric_limits<float>::max(),
                         "%.2f"))
      scale.x = scale.y = avgScale;
  }
  else
  {
    ImGui::PushItemWidth(ImGui::GetFontSize() * 8);
    ImGui::DragFloat("##X",
                     &scale.x,
                     0.01f,
                     -std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::max(),
                     "%.2f");
    ImGui::SameLine();
    ImGui::DragFloat("##Y",
                     &scale.y,
                     0.01f,
                     -std::numeric_limits<float>::max(),
                     std::numeric_limits<float>::max(),
                     "%.2f");
  }
  ImGui::SameLine();
  if (ImGui::Checkbox("Scale", &uniformScaling))
  {
    if (uniformScaling)
    {
      float avgScale = (scale.x + scale.y) / 2.0f;
      scale.x = scale.y = avgScale;
    }
  }
  ImGui::PopItemWidth();
  ImGui::PopID();

  ImGui::PushID("Pivot");
  ImGui::PushItemWidth(ImGui::GetFontSize() * 8);
  ImGui::DragFloat("##XPivot",
                   &pivot.x,
                   1.f,
                   -std::numeric_limits<float>::max(),
                   std::numeric_limits<float>::max(),
                   "%.1f");
  ImGui::SameLine();
  ImGui::DragFloat("Pivot",
                   &pivot.y,
                   1.f,
                   -std::numeric_limits<float>::max(),
                   std::numeric_limits<float>::max(),
                   "%.1f");
  ImGui::PopItemWidth();
  ImGui::PopID();

  ImGui::PushID("Rotation");
  ImGui::PushItemWidth(ImGui::GetFontSize() * 16 + 8);
  ImGui::SliderFloat("Rotation", &rot, -360.0f, 360.0f, "%.1f");
  ImGui::PopItemWidth();
  ImGui::PopID();
}

static glm::vec3 mouseToModelViewCoords(Vec2 mouse, Vec2 windowSize, const glm::mat4& mvpMatrix) {
    // Step 1: Normalize mouse coordinates to clip space coordinates
    glm::vec3 clipSpaceCoords(
        (2.0f * mouse.x) / windowSize.x - 1.0f, // X in range [-1, 1]
        1.0f - (2.0f * mouse.y) / windowSize.y, // Y in range [-1, 1], and invert the Y-axis
        1.0f // Z = 1, assuming the near plane is at Z = -1 in clip space
    );

    // Step 2: Unproject clip space coordinates to model view coordinates
    glm::mat4 invMVPMatrix = glm::inverse(mvpMatrix);
    glm::vec4 modelViewCoords = invMVPMatrix * glm::vec4(clipSpaceCoords, 1.0f);

    // Homogeneous division
    if (modelViewCoords.w != 0.0f) {
        modelViewCoords /= modelViewCoords.w;
    }

    return glm::vec3(modelViewCoords);
}

auto Node::screenToLocal(const glm::mat4 &projMat, Vec2 screen) const -> Vec2
{
  ImGuiIO &io = ImGui::GetIO();
  const auto w = io.DisplaySize.x;
  const auto h = io.DisplaySize.y;

  glm::mat4 mvpMatrix = projMat * modelViewMat;
  glm::vec3 modelViewCoords = mouseToModelViewCoords(screen, Vec2{w, h}, mvpMatrix);
  return Vec2{modelViewCoords.x, modelViewCoords.y};
}
