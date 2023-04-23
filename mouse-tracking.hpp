#pragma once
#include "mouse-sink.hpp"
#include <vector>

class MouseTracking
{
public:
  auto reg(MouseSink &) -> void;
  auto tick() -> void;
  auto unreg(MouseSink &) -> void;

private:
  std::vector<std::reference_wrapper<MouseSink>> mouseSinks;
};
