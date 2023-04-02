#include "node.hpp"
#include <SDL_opengl.h>
#include <algorithm>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <limits>
#include <log/log.hpp>

static auto getModelViewMatrix() -> glm::mat4
{
  GLfloat modelMatrixData[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, modelMatrixData);
  return glm::make_mat4(modelMatrixData);
}

auto Node::renderAll(Node *hovered, Node *selected) -> void
{
  glPushMatrix();
  // Apply transformations
  glTranslatef(loc.x, loc.y, 0.0f);       // Move the sprite
  glTranslatef(pivot.x, pivot.y, 0.0f);   // Move the pivot point
  glRotatef(rot, 0.0f, 0.0f, 1.0f);       // Rotate the sprite
  glScalef(scale.x, scale.y, 1.0f);       // Scale the sprite
  glTranslatef(-pivot.x, -pivot.y, 0.0f); // Move the pivot point back
  modelViewMat = getModelViewMatrix();

  render(hovered, selected);
  for (auto n : nodes_)
    n.get().renderAll(hovered, selected);
  glPopMatrix();
}

auto Node::renderUi() -> void
{
  auto n = name();
  ImGui::Text("# %s", n.c_str());
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
  {
    ImGui::PushStyleColor(
      ImGuiCol_Text,
      ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]); // Set text color to disabled color
    auto width = w();
    auto height = h();
    ImGui::InputFloat("##Width", &width, 0.f, 0.f, "%.1f", ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    ImGui::InputFloat("Size", &height, 0.f, 0.f, "%.1f", ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor(); // Restore the original text color
  }

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

static glm::vec3 mouseToModelViewCoords(glm::vec2 mouse,
                                        glm::vec2 windowSize,
                                        const glm::mat4 &mvpMatrix)
{
  // Step 1: Normalize mouse coordinates to clip space coordinates
  glm::vec3 clipSpaceCoords(
    (2.0f * mouse.x) / windowSize.x - 1.0f, // X in range [-1, 1]
    1.0f - (2.0f * mouse.y) / windowSize.y, // Y in range [-1, 1], and invert the Y-axis
    1.0f                                    // Z = 1, assuming the near plane is at Z = -1 in clip space
  );

  // Step 2: Unproject clip space coordinates to model view coordinates
  glm::mat4 invMVPMatrix = glm::inverse(mvpMatrix);
  glm::vec4 modelViewCoords = invMVPMatrix * glm::vec4(clipSpaceCoords, 1.0f);

  // Homogeneous division
  if (modelViewCoords.w != 0.0f)
  {
    modelViewCoords /= modelViewCoords.w;
  }

  return glm::vec3(modelViewCoords);
}

auto Node::screenToLocal(const glm::mat4 &projMat, glm::vec2 screen) const -> glm::vec2
{
  ImGuiIO &io = ImGui::GetIO();
  const auto w = io.DisplaySize.x;
  const auto h = io.DisplaySize.y;

  glm::mat4 mvpMatrix = projMat * modelViewMat;
  glm::vec3 modelViewCoords = mouseToModelViewCoords(screen, glm::vec2{w, h}, mvpMatrix);
  return glm::vec2{modelViewCoords.x, modelViewCoords.y};
}

auto Node::nodeUnder(const glm::mat4 &projMat, glm::vec2 v) -> Node *
{
  for (auto it = nodes_.rbegin(); it != nodes_.rend(); ++it)
  {
    auto u = it->get().nodeUnder(projMat, v);
    if (u)
      return u;
  }
  auto localPos = screenToLocal(projMat, v);
  if (localPos.x < 0.f || localPos.x > w() || localPos.y < 0.f || localPos.y > h())
    return nullptr;
  return this;
}

auto Node::render(Node *hovered, Node *selected) -> void
{
  if (selected != this && hovered != this)
    return;
  if (selected == this)
    glColor4f(1.f, .7f, .0f, 1.f);
  else if (hovered == this)
    glColor4f(1.f, 1.f, 1.0f, .2f);
  glBegin(GL_LINE_STRIP);
  glVertex2f(.0f, .0f);
  glVertex2f(w(), .0f);
  glVertex2f(w(), h());
  glVertex2f(.0f, h());
  glVertex2f(.0f, .0f);
  glEnd();
}

auto Node::updateLoc(const glm::mat4 &projMat,
                     glm::vec2 initLoc,
                     glm::vec2 startScreenLoc,
                     glm::vec2 endScreenLoc) -> void
{
  const auto startLoc = parent_ ? parent_->screenToLocal(projMat, startScreenLoc) : startScreenLoc;
  const auto endLoc = parent_ ? parent_->screenToLocal(projMat, endScreenLoc) : endScreenLoc;
  loc = initLoc + endLoc - startLoc;
}

auto Node::addChild(Node &v) -> void
{
  nodes_.push_back(v);
  v.parent_ = this;
}

auto Node::updateScale(const glm::mat4 &projMat,
                       glm::vec2 initScale,
                       glm::vec2 startScreenLoc,
                       glm::vec2 endScreenLoc) -> void
{
  const auto startLoc = screenToLocal(projMat, startScreenLoc);
  const auto endLoc = screenToLocal(projMat, endScreenLoc);

  // Calculate the scaling factor based on the distance between start and end locations
  const auto scaleFactor = glm::length(endLoc - pivot) / glm::length(startLoc - pivot);
  scale = initScale * scaleFactor;
}

auto Node::updateRot(const glm::mat4 &projMat,
                     float initRot,
                     glm::vec2 startScreenLoc,
                     glm::vec2 endScreenLoc) -> void
{
  const auto startLoc = screenToLocal(projMat, startScreenLoc);
  const auto endLoc = screenToLocal(projMat, endScreenLoc);

  // Calculate the angles between the pivot and start/end locations
  const auto startAngle =
    std::atan2(startLoc.y - pivot.y, startLoc.x - pivot.x) * 180.f / std::numbers::pi;
  const auto endAngle = std::atan2(endLoc.y - pivot.y, endLoc.x - pivot.x) * 180.f / std::numbers::pi;

  // Calculate the rotation difference and update the rotation
  const auto rotDiff = endAngle - startAngle;
  rot = initRot + rotDiff;
}

auto Node::nodes() const -> const std::vector<std::reference_wrapper<Node>> &
{
  return nodes_;
}

auto Node::moveUp() -> void
{
  if (!parent())
    return;
  if (&parent()->nodes_.front().get() == this)
    return;
  auto it = std::find_if(std::begin(parent()->nodes_),
                         std::end(parent()->nodes_),
                         [this](const auto &v) { return this == &v.get(); });
  assert(it != std::end(parent()->nodes_));
  auto prev = it - 1;
  std::swap(*it, *prev);
}

auto Node::moveDown() -> void
{
  if (!parent())
    return;
  if (!parent())
    return;
  if (&parent()->nodes_.back().get() == this)
    return;
  auto it = std::find_if(std::begin(parent()->nodes_),
                         std::end(parent()->nodes_),
                         [this](const auto &v) { return this == &v.get(); });
  assert(it != std::end(parent()->nodes_));
  auto prev = it + 1;
  std::swap(*it, *prev);
}

auto Node::unparent() -> void
{
  if (!parent())
    return;
  if (!parent()->parent())
    return;
  auto newParent = parent()->parent();
  auto it = std::find_if(std::begin(parent()->nodes_),
                         std::end(parent()->nodes_),
                         [this](const auto &v) { return this == &v.get(); });

  assert(it != std::end(parent()->nodes_));
  loc += parent()->loc;

  newParent->nodes_.push_back(*it);
  parent_->nodes_.erase(it);

  parent_ = newParent;
}

auto Node::parentWithBellow() -> void
{
  if (!parent())
    return;
  auto it = std::find_if(std::begin(parent()->nodes_),
                         std::end(parent()->nodes_),
                         [this](const auto &v) { return this == &v.get(); });

  assert(it != std::end(parent()->nodes_));

  if (&parent()->nodes_.back().get() == this)
    return;

  auto &nextSibling = *(it + 1);
  glm::mat4 nextSiblingTransform = nextSibling.get().modelViewMat;
  modelViewMat = glm::inverse(nextSiblingTransform) * modelViewMat;
  loc = glm::vec2{modelViewMat[3][0], modelViewMat[3][1]};

  nextSibling.get().nodes_.push_back(*it);
  parent_->nodes_.erase(it);

  parent_ = &nextSibling.get();
}
