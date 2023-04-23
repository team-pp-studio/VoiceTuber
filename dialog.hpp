#pragma once
#include <functional>
#include <string>

class Dialog
{
public:
  using Cb = std::function<auto(bool)->void>;
  Dialog(std::string title, Cb cb);
  virtual ~Dialog() = default;
  virtual auto draw() -> bool;

protected:
  Cb cb;

private:
  std::string title;
  bool first = true;
};
