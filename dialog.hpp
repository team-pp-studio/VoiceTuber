#pragma once
#include <functional>

class Dialog
{
public:
  using Cb = std::function<auto()->void>;
  Dialog(Cb cb);
  virtual ~Dialog() = default;
  virtual auto draw() -> bool = 0;

protected:
  Cb cb;
};
