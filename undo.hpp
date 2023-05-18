#pragma once
#include <SDL.h>
#include <functional>
#include <string>
#include <vector>

class Undo
{
public:
  using Op = std::function<auto()->void>;
  struct Action
  {
    Action(std::string aTag, Uint32 aTimestamp, Op aAction, Op aRollback)
      : tag(std::move(aTag)),
        timestamp(aTimestamp),
        action(std::move(aAction)),
        rollback(std::move(aRollback))
    {
    }
    std::string tag;
    Uint32 timestamp;
    Op action;
    Op rollback;
  };
  auto record(Op action, Op rollback, std::string tag = "") -> void;
  auto undo() -> void;
  auto redo() -> void;
  auto hasUndo() const -> bool;
  auto hasRedo() const -> bool;

private:
  std::vector<Action> undoStack;
  std::vector<Action> redoStack;
};
