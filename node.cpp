#include "node.hpp"
#include "save-factory.hpp"
#include <SDL_opengl.h>
#include <algorithm>
#include <glm/glm.hpp>
#include <limits>
#include <log/log.hpp>
#include <numbers>

namespace Internal
{
  auto serVal(OStrm &strm, const glm::vec2 &value) noexcept -> void
  {
    strm.write(reinterpret_cast<const char *>(&value), sizeof(value));
  }

  auto deserVal(IStrm &strm, glm::vec2 &value) noexcept -> void
  {
    strm.read(reinterpret_cast<char *>(&value), sizeof(value));
  }

  auto serVal(OStrm &strm, const ImVec4 &value) noexcept -> void
  {
    strm.write(reinterpret_cast<const char *>(&value), sizeof(value));
  }

  auto deserVal(IStrm &strm, ImVec4 &value) noexcept -> void
  {
    strm.read(reinterpret_cast<char *>(&value), sizeof(value));
  }
} // namespace Internal

static auto getModelViewMatrix() -> glm::mat4
{
  GLfloat modelMatrixData[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, modelMatrixData);
  return glm::make_mat4(modelMatrixData);
}

Node::Node(std::string name) : name(std::move(name)) {}

auto Node::renderAll(float dt, Node *hovered, Node *selected) -> void
{
  glPushMatrix();
  // Apply transformations
  glTranslatef(loc.x, loc.y, 0.0f);           // Move the sprite
  glRotatef(rot + animRot, 0.0f, 0.0f, 1.0f); // Rotate the sprite
  glScalef(scale.x, scale.y, 1.0f);           // Scale the sprite
  glTranslatef(-pivot.x, -pivot.y, 0.0f);     // Move the pivot point back
  modelViewMat = getModelViewMatrix();

  render(dt, hovered, selected);
  for (auto &n : nodes)
    n->renderAll(dt, hovered, selected);
  glPopMatrix();
}

auto Node::renderUi() -> void
{
  ImGui::Text("# %s", name.c_str());
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
  if (ImGui::Button("NW"))
  {
    pivot = glm::vec2{0, h()};
  }
  ImGui::SameLine();
  if (ImGui::Button("N "))
  {
    pivot = glm::vec2{w() / 2, h()};
  }
  ImGui::SameLine();
  if (ImGui::Button("NE"))
  {
    pivot = glm::vec2{w(), h()};
  }
  if (ImGui::Button("W "))
  {
    pivot = glm::vec2{w(), h() / 2};
  }
  ImGui::SameLine();
  if (ImGui::Button("C "))
  {
    pivot = glm::vec2{w() / 2, h() / 2};
  }
  ImGui::SameLine();
  if (ImGui::Button("E "))
  {
    pivot = glm::vec2{w(), h() / 2};
  }
  if (ImGui::Button("SW"))
  {
    pivot = glm::vec2{0, 0};
  }
  ImGui::SameLine();
  if (ImGui::Button("S "))
  {
    pivot = glm::vec2{w() / 2, 0};
  }
  ImGui::SameLine();
  if (ImGui::Button("SE"))
  {
    pivot = glm::vec2{w(), 0};
  }
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
  for (auto it = nodes.rbegin(); it != nodes.rend(); ++it)
  {
    auto u = (*it)->nodeUnder(projMat, v);
    if (u)
      return u;
  }
  auto localPos = screenToLocal(projMat, v);
  if (localPos.x < 0.f || localPos.x > w() || localPos.y < 0.f || localPos.y > h() ||
      isTransparent(localPos))
    return nullptr;
  return this;
}

auto Node::render(float /*dt*/, Node *hovered, Node *selected) -> void
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
  const auto d = std::max(w(), h()) * .05f;
  if (selected == this)
  {
    glBegin(GL_LINES);
    glVertex2f(pivot.x - d, pivot.y - d);
    glVertex2f(pivot.x + d, pivot.y + d);
    glVertex2f(pivot.x - d, pivot.y + d);
    glVertex2f(pivot.x + d, pivot.y - d);
    glEnd();
  }
}

auto Node::addChild(std::unique_ptr<Node> v) -> void
{
  v->parent_ = this;
  nodes.emplace_back(std::move(v));
}

auto Node::getNodes() const -> const Nodes &
{
  return nodes;
}

auto Node::moveUp() -> void
{
  if (!parent())
    return;
  if (parent()->nodes.front().get() == this)
    return;
  auto it = std::find_if(std::begin(parent()->nodes), std::end(parent()->nodes), [this](const auto &v) {
    return this == v.get();
  });
  assert(it != std::end(parent()->nodes));
  auto prev = it - 1;
  std::swap(*it, *prev);
}

auto Node::moveDown() -> void
{
  if (!parent())
    return;
  if (!parent())
    return;
  if (parent()->nodes.back().get() == this)
    return;
  auto it = std::find_if(std::begin(parent()->nodes), std::end(parent()->nodes), [this](const auto &v) {
    return this == v.get();
  });
  assert(it != std::end(parent()->nodes));
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
  auto it = std::find_if(std::begin(parent()->nodes), std::end(parent()->nodes), [this](const auto &v) {
    return this == v.get();
  });

  assert(it != std::end(parent()->nodes));
  loc += parent()->loc;
  newParent->nodes.emplace_back(std::move(*it));
  parent_->nodes.erase(it);
  parent_ = newParent;
}

auto Node::parentWithBellow() -> void
{
  if (!parent())
    return;
  auto it = std::find_if(std::begin(parent()->nodes), std::end(parent()->nodes), [this](const auto &v) {
    return this == v.get();
  });

  assert(it != std::end(parent()->nodes));

  if (parent()->nodes.back().get() == this)
    return;

  auto nextSibling = (it + 1)->get();
  glm::mat4 nextSiblingTransform = nextSibling->modelViewMat;
  modelViewMat = glm::inverse(nextSiblingTransform) * modelViewMat;
  loc = glm::vec2{modelViewMat[3][0], modelViewMat[3][1]};

  nextSibling->nodes.emplace_back(std::move(*it));
  parent_->nodes.erase(it);
  parent_ = nextSibling;
}

auto Node::del(Node &node) -> void
{
  if (!node.parent_)
    return;
  auto &parentNodes = node.parent_->nodes;
  auto it = std::find_if(
    parentNodes.begin(), parentNodes.end(), [&node](const auto &v) { return &node == v.get(); });
  assert(it != parentNodes.end());
  parentNodes.erase(it);
}

auto Node::translateCancel() -> void
{
  loc = initLoc;
}

auto Node::translateStart(glm::vec2 mouse) -> void
{
  startMousePos = mouse;
  initLoc = loc;
}

auto Node::translateUpdate(const glm::mat4 &projMat, glm::vec2 mouse) -> void
{
  const auto startLoc = parent_ ? parent_->screenToLocal(projMat, startMousePos) : startMousePos;
  const auto endLoc = parent_ ? parent_->screenToLocal(projMat, mouse) : mouse;
  loc = initLoc + endLoc - startLoc;
}

auto Node::rotCancel() -> void
{
  rot = initRot;
}

auto Node::rotStart(glm::vec2 mouse) -> void
{
  startMousePos = mouse;
  initRot = rot;
}

auto Node::rotUpdate(const glm::mat4 &projMat, glm::vec2 mouse) -> void
{
  const auto startLoc = screenToLocal(projMat, startMousePos);
  const auto endLoc = screenToLocal(projMat, mouse);

  // Calculate the angles between the pivot and start/end locations
  const auto startAngle =
    std::atan2(startLoc.y - pivot.y, startLoc.x - pivot.x) * 180.f / std::numbers::pi;
  const auto endAngle = std::atan2(endLoc.y - pivot.y, endLoc.x - pivot.x) * 180.f / std::numbers::pi;

  // Calculate the rotation difference and update the rotation
  const auto rotDiff = endAngle - startAngle;
  rot = initRot + rotDiff;
}

auto Node::scaleCancel() -> void
{
  scale = initScale;
}

auto Node::scaleStart(glm::vec2 mouse) -> void
{
  startMousePos = mouse;
  initScale = scale;
}

auto Node::scaleUpdate(const glm::mat4 &projMat, glm::vec2 mouse) -> void
{
  const auto startLoc = screenToLocal(projMat, startMousePos);
  const auto endLoc = screenToLocal(projMat, mouse);

  // Calculate the scaling factor based on the distance between start and end locations
  const auto scaleFactor = glm::length(endLoc - pivot) / glm::length(startLoc - pivot);
  scale = initScale * scaleFactor;
}

auto Node::saveAll(OStrm &strm) const -> void
{
  save(strm);
  auto sz = static_cast<int32_t>(nodes.size());
  ::ser(strm, sz);
  for (const auto &n : nodes)
    n->saveAll(strm);
}

auto Node::loadAll(const class SaveFactory &saveFactory, IStrm &strm) -> void
{
  load(strm);
  int32_t sz;
  ::deser(strm, sz);
  for (auto i = 0; i < sz; ++i)
  {
    std::string className;
    std::string name;
    ::deser(strm, className);
    ::deser(strm, name);
    auto node = saveFactory.ctor(className, name);
    node->loadAll(saveFactory, strm);
    addChild(std::move(node));
  }
}

auto Node::save(OStrm &strm) const -> void
{
  ::ser(strm, *this);
}

auto Node::load(IStrm &strm) -> void
{
  ::deser(strm, *this);
}

auto Node::isTransparent(glm::vec2) const -> bool
{
  return false;
}

auto Node::h() const -> float
{
  return 1.f;
}

auto Node::w() const -> float
{
  return 1.f;
}

auto Node::parent() -> Node *
{
  return parent_;
}

auto Node::getName() const -> std::string
{
  return name;
}
