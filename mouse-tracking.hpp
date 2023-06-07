#pragma once
#include "mouse-sink.hpp"
#include "uv.hpp"
#include <vector>

class MouseTracking
{
public:
  MouseTracking(uv::Uv &);
  auto reg(MouseSink &) -> void;
  auto unreg(MouseSink &) -> void;

private:
  uv::Prepare prepare;
  std::vector<std::reference_wrapper<MouseSink>> mouseSinks;
  auto tick() -> void;
};
