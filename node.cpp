#include "node.hpp"
#include <SDL_opengl.h>
#include <imgui/imgui.h>
#include <limits>

auto Node::renderAll() -> void
{
  glPushMatrix();
  // Apply transformations
  glTranslatef(loc.x, loc.y, 0.0f);       // Move the sprite
  glTranslatef(pivot.x, pivot.y, 0.0f);   // Move the pivot point
  glRotatef(rot, 0.0f, 0.0f, 1.0f);       // Rotate the sprite
  glScalef(scale.x, scale.y, 1.0f);       // Scale the sprite
  glTranslatef(-pivot.x, -pivot.y, 0.0f); // Move the pivot point back

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
