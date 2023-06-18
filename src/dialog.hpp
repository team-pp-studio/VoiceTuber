#pragma once
#include <functional>
#include <memory>
#include <string>

class Dialog
{
public:
  using Cb = std::function<auto(bool)->void>;
  Dialog(std::string title, Cb cb);
  virtual ~Dialog() = default;
  auto draw() -> bool;

protected:
  enum class DialogState { active, ok, cancel };
  virtual auto internalDraw() -> DialogState = 0;

private:
  Cb cb;
  std::string title;
  bool first = true;
};
