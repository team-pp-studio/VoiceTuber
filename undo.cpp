#include "undo.hpp"

auto Undo::record(Op op, Op undo) -> void
{
  op();
  undoStack.emplace_back(std::move(op), std::move(undo));
  redoStack.clear();
}

auto Undo::undo() -> void
{
  if (!hasUndo())
    return;
  undoStack.back().second();
  redoStack.emplace_back(std::move(undoStack.back()));
  undoStack.pop_back();
}

auto Undo::redo() -> void
{
  if (!hasRedo())
    return;
  redoStack.back().first();
  undoStack.emplace_back(std::move(redoStack.back()));
  redoStack.pop_back();
}

auto Undo::hasUndo() const -> bool
{
  return !undoStack.empty();
}

auto Undo::hasRedo() const -> bool
{
  return !redoStack.empty();
}
