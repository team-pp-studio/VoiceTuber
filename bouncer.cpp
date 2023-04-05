#include "bouncer.hpp"
#include "audio-capture.hpp"
#include <SDL_opengl.h>
#include <limits>
#include <log/log.hpp>

Bouncer::Bouncer(class AudioCapture &audioCapture) : Node("bouncer"), audioCapture(audioCapture)
{
  audioCapture.reg(*this);
}

Bouncer::~Bouncer()
{
  audioCapture.get().unreg(*this);
}

auto Bouncer::ingest(Wav v) -> void
{
  if (v.empty())
    return;
  offset = strength * v.back() / 0x8000;
}

auto Bouncer::render(float dt, Node *hovered, Node *selected) -> void
{
  glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
  glClear(GL_COLOR_BUFFER_BIT);
  glTranslatef(.0f, offset, .0f);
  Node::render(dt, hovered, selected);
}

auto Bouncer::renderUi() -> void
{
  ImGui::PushID("Bouncer");
  ImGui::PushItemWidth(ImGui::GetFontSize() * 16 + 8);
  ImGui::ColorEdit4("BG color", (float *)&clearColor); // Edit 3 floats representing a color
  ImGui::DragFloat("Bounce",
                   &strength,
                   1.f,
                   0.f,
                   std::numeric_limits<float>::max(),
                   "%.1f",
                   ImGuiSliderFlags_AlwaysClamp);
  ImGui::PopItemWidth();
  ImGui::PopID();
}

auto Bouncer::save(OStrm &strm) const -> void
{
  ::ser(strm, className);
  ::ser(strm, name);
  ::ser(strm, *this);
  Node::save(strm);
}
auto Bouncer::load(IStrm &strm) -> void
{
  ::deser(strm, *this);
  Node::load(strm);
}
