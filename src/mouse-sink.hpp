#pragma once
#include <glm/glm.hpp>

class MouseSink
{
public:
  virtual ~MouseSink() = default;
  virtual auto ingest(const glm::mat4 &projMat, glm::vec2) -> void = 0;
};
