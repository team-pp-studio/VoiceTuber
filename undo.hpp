#pragma once
#include <functional>
#include <vector>

class Undo
{
public:
  using Op = std::function<auto()->void>;
  auto record(Op op, Op undo) -> void;
  auto undo() -> void;
  auto redo() -> void;
  auto hasUndo() const -> bool;
  auto hasRedo() const -> bool;

private:
  std::vector<std::pair<Op, Op>> undoStack;
  std::vector<std::pair<Op, Op>> redoStack;
};
