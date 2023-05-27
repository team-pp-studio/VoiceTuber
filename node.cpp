#include "node.hpp"
#include "save-factory.hpp"
#include "ui.hpp"
#include "undo.hpp"
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

static auto setModelViewMatrix(glm::mat4 v) -> void
{
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(glm::value_ptr(v));
}

Node::Node(Lib &lib, Undo &undo, std::string name)
  : name(std::move(name)),
    undo(undo),
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

auto Node::renderAll(float dt, Node *hovered, Node *selected) -> void
{
  auto ns = Nodes{};
  getAllNodesCalcModelView(ns);

  std::stable_sort(std::begin(ns), std::end(ns), [](const auto a, const auto b) {
    return a.get().zOrder < b.get().zOrder;
  });

  glPushMatrix();
  for (auto &n : ns)
  {
    setModelViewMatrix(n.get().modelViewMat);
    n.get().render(dt, hovered, selected);
  }
  glPopMatrix();
}

auto Node::getAllNodesCalcModelView(Nodes &out) -> void
{
  glPushMatrix();
  // Apply transformations
  glTranslatef(loc.x, loc.y, 0.0f);           // Move the sprite
  glRotatef(rot + animRot, 0.0f, 0.0f, 1.0f); // Rotate the sprite
  glScalef(scale.x, scale.y, 1.0f);           // Scale the sprite
  glTranslatef(-pivot_.x, -pivot_.y, 0.0f);   // Move the pivot point back
  modelViewMat = getModelViewMatrix();

  out.push_back(*this);
  for (auto &n : nodes)
    n->getAllNodesCalcModelView(out);
  glPopMatrix();
}

auto Node::renderUi() -> void
{
  ImGui::TableNextColumn();
  ImGui::Text("#");
  ImGui::TableNextColumn();
  ImGui::Text("%s", name.c_str());
  ImGui::TableNextColumn();
  Ui::textRj("Location");
  ImGui::TableNextColumn();

  Ui::dragFloat(undo,
                "X##XLoc",
                loc.x,
                1.f,
                -std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                "%.1f");
  Ui::dragFloat(undo,
                "Y##YLoc",
                loc.y,
                1.f,
                -std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                "%.1f");
  {
    ImGui::TableNextColumn();
    Ui::textRj("Size");
    ImGui::TableNextColumn();
    ImGui::PushStyleColor(
      ImGuiCol_Text,
      ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]); // Set text color to disabled color
    auto width = w();
    auto height = h();
    ImGui::InputFloat("W##Width", &width, 0.f, 0.f, "%.1f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat("H##Height", &height, 0.f, 0.f, "%.1f", ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor(); // Restore the original text color
  }

  ImGui::TableNextColumn();
  Ui::textRj("Scale");
  ImGui::TableNextColumn();
  if (uniformScaling)
  {
    {
      float avgScale = (scale.x + scale.y) / 2.0f;
      if (ImGui::DragFloat("##X",
                           &avgScale,
                           0.01f,
                           -std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max(),
                           "%.2f"))
      {
        undo.get().record([avgScale, this]() { scale.x = scale.y = avgScale; },
                          [oldScale = scale, this]() { scale = oldScale; },
                          "Scale");
      }
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("##Scale", &uniformScaling))
    {
      if (uniformScaling)
      {
        float avgScale = (scale.x + scale.y) / 2.0f;
        scale.x = scale.y = avgScale;
      }
    }
  }
  else
  {
    Ui::dragFloat(undo,
                  "X##X",
                  scale.x,
                  0.01f,
                  -std::numeric_limits<float>::max(),
                  std::numeric_limits<float>::max(),
                  "%.2f");
    ImGui::SameLine();
    if (ImGui::Checkbox("##Scale", &uniformScaling))
    {
      if (uniformScaling)
      {
        float avgScale = (scale.x + scale.y) / 2.0f;
        scale.x = scale.y = avgScale;
      }
    }
    Ui::dragFloat(undo,
                  "Y##Y",
                  scale.y,
                  0.01f,
                  -std::numeric_limits<float>::max(),
                  std::numeric_limits<float>::max(),
                  "%.2f");
  }
  ImGui::TableNextColumn();
  Ui::textRj("ZOrder");
  ImGui::TableNextColumn();
  Ui::inputInt(undo, "##ZOrder", zOrder);
  ImGui::TableNextColumn();
  Ui::textRj("Pivot");
  ImGui::TableNextColumn();
  Ui::dragFloat(undo,
                "X##XPivot",
                pivot_.x,
                1.f,
                -std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                "%.1f");
  Ui::dragFloat(undo,
                "Y##YPivot",
                pivot_.y,
                1.f,
                -std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                "%.1f");
  const auto sz = 2 * ImGui::GetFontSize();
  if (Ui::btnImg("nw", *arrowNW, sz, sz))
    undo.get().record([newPivot = glm::vec2{0, h()}, this]() { pivot_ = newPivot; },
                      [oldPivot = pivot_, this]() { pivot_ = oldPivot; });
  ImGui::SameLine();
  if (Ui::btnImg("n", *arrowN, sz, sz))
    undo.get().record([newPivot = glm::vec2{w() / 2, h()}, this]() { pivot_ = newPivot; },
                      [oldPivot = pivot_, this]() { pivot_ = oldPivot; });
  ImGui::SameLine();
  if (Ui::btnImg("ne", *arrowNE, sz, sz))
    undo.get().record([newPivot = glm::vec2{w(), h()}, this]() { pivot_ = newPivot; },
                      [oldPivot = pivot_, this]() { pivot_ = oldPivot; });
  if (Ui::btnImg("w", *arrowW, sz, sz))
    undo.get().record([newPivot = glm::vec2{0, h() / 2}, this]() { pivot_ = newPivot; },
                      [oldPivot = pivot_, this]() { pivot_ = oldPivot; });
  ImGui::SameLine();
  if (Ui::btnImg("c", *center, sz, sz))
    undo.get().record([newPivot = glm::vec2{w() / 2, h() / 2}, this]() { pivot_ = newPivot; },
                      [oldPivot = pivot_, this]() { pivot_ = oldPivot; });
  ImGui::SameLine();
  if (Ui::btnImg("e", *arrowE, sz, sz))
    undo.get().record([newPivot = glm::vec2{w(), h() / 2}, this]() { pivot_ = newPivot; },
                      [oldPivot = pivot_, this]() { pivot_ = oldPivot; });
  if (Ui::btnImg("sw", *arrowSW, sz, sz))
    undo.get().record([newPivot = glm::vec2{0, 0}, this]() { pivot_ = newPivot; },
                      [oldPivot = pivot_, this]() { pivot_ = oldPivot; });
  ImGui::SameLine();
  if (Ui::btnImg("s", *arrowS, sz, sz))
    undo.get().record([newPivot = glm::vec2{w() / 2, 0}, this]() { pivot_ = newPivot; },
                      [oldPivot = pivot_, this]() { pivot_ = oldPivot; });
  ImGui::SameLine();
  if (Ui::btnImg("se", *arrowSE, sz, sz))
    undo.get().record([newPivot = glm::vec2{w(), 0}, this]() { pivot_ = newPivot; },
                      [oldPivot = pivot_, this]() { pivot_ = oldPivot; });
  ImGui::TableNextColumn();
  Ui::textRj("Rotation");
  ImGui::TableNextColumn();
  Ui::dragFloat(undo, "°##Rotation", rot, 1.f, -360.0f, 360.0f, "%.1f");
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

auto Node::localToScreen(const glm::mat4 &projMat, glm::vec2 local) const -> glm::vec2
{
  ImGuiIO &io = ImGui::GetIO();
  const auto w = io.DisplaySize.x;
  const auto h = io.DisplaySize.y;

  glm::mat4 mvpMatrix = projMat * modelViewMat;

  glm::vec4 clipSpaceCoords = mvpMatrix * glm::vec4(local, 0.0f, 1.0f);
  clipSpaceCoords /= clipSpaceCoords.w; // Perspective division

  // Convert from clip space (-1.0 to 1.0) to screen space
  glm::vec2 screenCoords = glm::vec2(clipSpaceCoords) * 0.5f + 0.5f;
  screenCoords.x *= w;
  screenCoords.y *= h;

  return screenCoords;
}

auto Node::nodeUnder(const glm::mat4 &projMat, glm::vec2 v) -> Node *
{
  Nodes underNodes;
  collectUnderNodes(projMat, v, underNodes);

  std::stable_sort(underNodes.begin(), underNodes.end(), [](const auto a, const auto b) {
    return a.get().zOrder > b.get().zOrder;
  });

  if (!underNodes.empty())
    return &underNodes.front().get();
  return nullptr;
}

auto Node::collectUnderNodes(const glm::mat4 &projMat, glm::vec2 v, Nodes &underNodes) -> void
{
  for (auto it = nodes.rbegin(); it != nodes.rend(); ++it)
    (*it)->collectUnderNodes(projMat, v, underNodes);

  auto localPos = screenToLocal(projMat, v);
  if (!(localPos.x < 0.f || localPos.x > w() || localPos.y < 0.f || localPos.y > h() ||
        isTransparent(localPos)))
    underNodes.push_back(*this);
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
  if (selected != this)
    return;
  glBegin(GL_LINES);
  glVertex2f(pivot_.x - d, pivot_.y - d);
  glVertex2f(pivot_.x + d, pivot_.y + d);
  glVertex2f(pivot_.x - d, pivot_.y + d);
  glVertex2f(pivot_.x + d, pivot_.y - d);
  glEnd();
}

auto Node::addChild(std::shared_ptr<Node> v) -> void
{
  v->parent_ = this;
  nodes.emplace_back(std::move(v));
}

auto Node::getNodes() const -> const PNodes &
{
  return nodes;
}

auto Node::moveUp() -> void
{
  if (!parent())
    return;
  if (parent()->nodes.front().get() == this)
    return;
  undo.get().record(
    [this]() {
      auto it = std::find_if(std::begin(parent()->nodes),
                             std::end(parent()->nodes),
                             [this](const auto &v) { return this == v.get(); });
      assert(it != std::end(parent()->nodes));
      auto prev = it - 1;
      std::swap(*it, *prev);
    },
    [this]() {
      auto it = std::find_if(std::begin(parent()->nodes),
                             std::end(parent()->nodes),
                             [this](const auto &v) { return this == v.get(); });
      assert(it != std::end(parent()->nodes));
      auto prev = it + 1;
      std::swap(*it, *prev);
    });
}

auto Node::moveDown() -> void
{
  if (!parent())
    return;
  if (!parent())
    return;
  if (parent()->nodes.back().get() == this)
    return;
  undo.get().record(
    [this]() {
      auto it = std::find_if(std::begin(parent()->nodes),
                             std::end(parent()->nodes),
                             [this](const auto &v) { return this == v.get(); });
      assert(it != std::end(parent()->nodes));
      auto prev = it + 1;
      std::swap(*it, *prev);
    },
    [this]() {
      auto it = std::find_if(std::begin(parent()->nodes),
                             std::end(parent()->nodes),
                             [this](const auto &v) { return this == v.get(); });
      assert(it != std::end(parent()->nodes));
      auto prev = it - 1;
      std::swap(*it, *prev);
    });
}

auto Node::unparent() -> void
{
  if (!parent())
    return;
  if (!parent()->parent())
    return;

  auto newParent = parent()->parent();
  auto oldParent = parent();
  auto it = std::find_if(std::begin(oldParent->nodes),
                         std::end(oldParent->nodes),
                         [this](const auto &v) { return this == v.get(); });
  assert(it != std::end(oldParent->nodes));
  auto self = std::move(*it);
  undo.get().record(
    [it, newParent, oldParent, this, self]() {
      loc += oldParent->loc;
      newParent->nodes.emplace_back(std::move(self));
      oldParent->nodes.erase(it);
      parent_ = newParent;
    },
    [it, newParent, oldLoc = loc, oldParent, this, self]() {
      loc = oldLoc;
      auto it2 = std::find_if(std::begin(newParent->nodes),
                              std::end(newParent->nodes),
                              [this](const auto &v) { return this == v.get(); });
      assert(it2 != std::end(newParent->nodes));
      newParent->nodes.erase(it2);
      oldParent->nodes.emplace(it, std::move(self));
      parent_ = oldParent;
    });
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

  auto newParent = (it + 1)->get();
  auto self = std::move(*it);
  auto oldParent = parent();
  undo.get().record(
    [newParent, this, it, self]() {
      glm::mat4 newParentTransform = newParent->modelViewMat;
      modelViewMat = glm::inverse(newParentTransform) * modelViewMat;
      loc = glm::vec2{modelViewMat[3][0], modelViewMat[3][1]};

      newParent->nodes.emplace_back(std::move(self));
      parent_->nodes.erase(it);
      parent_ = newParent;
    },
    [it, newParent, oldLoc = loc, oldParent, this, self]() {
      loc = oldLoc;
      auto it2 = std::find_if(std::begin(newParent->nodes),
                              std::end(newParent->nodes),
                              [this](const auto &v) { return this == v.get(); });
      assert(it2 != std::end(newParent->nodes));
      newParent->nodes.erase(it2);
      oldParent->nodes.emplace(it, std::move(self));
      parent_ = oldParent;
    });
}

auto Node::del(Node **ppNode) -> void
{
  if (!*ppNode)
    return;
  auto pNode = *ppNode;
  if (!pNode->parent_)
    return;
  auto &undo = pNode->undo.get();
  auto &parentNodes = pNode->parent_->nodes;
  auto it = std::find_if(
    parentNodes.begin(), parentNodes.end(), [&pNode](const auto &v) { return pNode == v.get(); });
  assert(it != parentNodes.end());
  undo.record(
    [&parentNodes, it, ppNode]() {
      parentNodes.erase(it);
      *ppNode = nullptr;
    },
    [&parentNodes, it, ppNode, spNode = std::move(*it)]() mutable {
      *ppNode = spNode.get();
      parentNodes.emplace(it, std::move(spNode));
    });
}

auto Node::delNoUndo(Node &node) -> void
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
  editMode_ = EditMode::translate;
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
  editMode_ = EditMode::rotate;
}

auto Node::rotUpdate(const glm::mat4 &projMat, glm::vec2 mouse) -> void
{
  const auto startLoc = screenToLocal(projMat, startMousePos);
  const auto endLoc = screenToLocal(projMat, mouse);

  // Calculate the angles between the pivot and start/end locations
  const auto startAngle =
    atan2f(startLoc.y - pivot_.y, startLoc.x - pivot_.x) * 180.f / std::numbers::pi_v<float>;
  const auto endAngle =
    atan2f(endLoc.y - pivot_.y, endLoc.x - pivot_.x) * 180.f / std::numbers::pi_v<float>;

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
  editMode_ = EditMode::scale;
}

auto Node::scaleUpdate(const glm::mat4 &projMat, glm::vec2 mouse) -> void
{
  const auto startLoc = screenToLocal(projMat, startMousePos);
  const auto endLoc = screenToLocal(projMat, mouse);

  // Calculate the scaling factor based on the distance between start and end locations
  const auto scaleFactor = glm::length(endLoc - pivot()) / glm::length(startLoc - pivot());
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
    std::string n;
    ::deser(strm, className);
    ::deser(strm, n);
    auto node = saveFactory.ctor(className, n);
    if (!node)
    {
      LOG("class name", className, "name", n);
      assert(node);
    }
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

auto Node::update(const glm::mat4 &projMat, glm::vec2 mouse) -> void
{
  switch (editMode_)
  {
  case EditMode::translate: translateUpdate(projMat, mouse); break;
  case EditMode::rotate: rotUpdate(projMat, mouse); break;
  case EditMode::scale: scaleUpdate(projMat, mouse); break;
  case EditMode::select: break;
  }
}
auto Node::cancel() -> void
{
  switch (editMode_)
  {
  case EditMode::translate: translateCancel(); break;
  case EditMode::rotate: rotCancel(); break;
  case EditMode::scale: scaleCancel(); break;
  case EditMode::select: break;
  }
  editMode_ = EditMode::select;
}

auto Node::commit() -> void
{
  switch (editMode_)
  {
  case EditMode::translate:
    undo.get().record([newLoc = loc, this]() { loc = newLoc; },
                      [oldLoc = initLoc, this]() { loc = oldLoc; });
    break;
  case EditMode::rotate:
    undo.get().record([newRot = rot, this]() { rot = newRot; },
                      [oldRot = initRot, this]() { rot = oldRot; });
    break;
  case EditMode::scale:
    undo.get().record([newScale = scale, this]() { scale = newScale; },
                      [oldScale = initScale, this]() { scale = oldScale; });
    break;
  case EditMode::select: break;
  }

  editMode_ = EditMode::select;
}

auto Node::editMode() const -> EditMode
{
  return editMode_;
}

auto Node::pivot() const -> glm::vec2
{
  return pivot_;
}

auto Node::parentWith(Node &newParent) -> void
{
  if (!parent())
    return;
  auto it = std::find_if(std::begin(parent()->nodes), std::end(parent()->nodes), [this](const auto &v) {
    return this == v.get();
  });

  assert(it != std::end(parent()->nodes));

  auto self = std::move(*it);
  auto oldParent = parent();
  undo.get().record(
    [&newParent, this, it, self]() {
      glm::mat4 newParentTransform = newParent.modelViewMat;
      modelViewMat = glm::inverse(newParentTransform) * modelViewMat;
      loc = glm::vec2{modelViewMat[3][0], modelViewMat[3][1]};

      newParent.nodes.emplace_back(std::move(self));
      parent_->nodes.erase(it);
      parent_ = &newParent;
    },
    [it, &newParent, oldLoc = loc, oldParent, this, self]() {
      loc = oldLoc;
      auto it2 = std::find_if(std::begin(newParent.nodes),
                              std::end(newParent.nodes),
                              [this](const auto &v) { return this == v.get(); });
      assert(it2 != std::end(newParent.nodes));
      newParent.nodes.erase(it2);
      oldParent->nodes.emplace(it, std::move(self));
      parent_ = oldParent;
    });
}

auto Node::placeBellow(Node &newSibling) -> void
{
  assert(newSibling.parent());
  undo.get().record(
    [this, &newSibling]() {
      auto selfIt = std::find_if(std::begin(parent()->nodes),
                                 std::end(parent()->nodes),
                                 [this](const auto &v) { return this == v.get(); });
      auto self = std::move(*selfIt);
      parent()->nodes.erase(selfIt);
      self->parent_ = newSibling.parent();
      auto newSiblingIt = std::find_if(std::begin(newSibling.parent()->nodes),
                                       std::end(newSibling.parent()->nodes),
                                       [&newSibling](const auto &v) { return &newSibling == v.get(); });
      assert(newSiblingIt != std::end(newSibling.parent()->nodes));
      ++newSiblingIt;
      newSibling.parent()->nodes.insert(newSiblingIt, std::move(self));
    },
    [this, nodes = parent()->nodes, oldParent = parent()]() {
      auto selfIt = std::find_if(std::begin(parent()->nodes),
                                 std::end(parent()->nodes),
                                 [this](const auto &v) { return this == v.get(); });
      assert(selfIt != std::end(parent()->nodes));
      parent()->nodes.erase(selfIt);
      oldParent->nodes = nodes;
      parent_ = oldParent;
    });
}
