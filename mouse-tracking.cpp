#include "mouse-tracking.hpp"
#include <SDL_opengl.h>
#include <glm/gtc/type_ptr.hpp>
#include <sdlpp/sdlpp.hpp>

static auto getProjMat() -> glm::mat4
{
  GLfloat projMatData[16];
  glGetFloatv(GL_PROJECTION_MATRIX, projMatData);
  return glm::make_mat4(projMatData);
}

auto MouseTracking::tick() -> void
{
  const auto projMat = getProjMat();
  int x, y;
  SDL_GetGlobalMouseState(&x, &y);
  for (auto mouseSink : mouseSinks)
    mouseSink.get().ingest(projMat, glm::vec2{1.f * x, 1.f * y});
}

auto MouseTracking::reg(MouseSink &v) -> void
{
  mouseSinks.push_back(v);
}

auto MouseTracking::unreg(MouseSink &v) -> void
{
  mouseSinks.erase(std::remove_if(std::begin(mouseSinks),
                                  std::end(mouseSinks),
                                  [&](const auto &x) { return &x.get() == &v; }),
                   std::end(mouseSinks));
}
