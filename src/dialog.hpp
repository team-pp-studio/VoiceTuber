#pragma once
#include <functional>
#include <memory>
#include <string>

class Dialog
{
public:
  using Callback = std::move_only_function<void(bool)>;
  Dialog(std::string title, Callback callback);
  virtual ~Dialog();

  bool draw();

protected:
  enum class DialogState { active, ok, cancel };
  virtual auto internalDraw() -> DialogState = 0;

private:
  Callback callback;
  std::string title;
  bool first = true;
};
