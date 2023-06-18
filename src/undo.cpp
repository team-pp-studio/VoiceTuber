#include "undo.hpp"
#include <cmath>

auto Undo::record(Op action, Op rollback, std::string tag) -> void
{
  action();
  undoStack.emplace_back(std::move(tag), SDL_GetTicks(), std::move(action), std::move(rollback));
  redoStack.clear();
}

auto Undo::undo() -> void
{
  if (!hasUndo())
    return;
  std::string lastTag;
  Uint32 lastTimestamp;
  do
  {
    auto &rec = undoStack.back();
    rec.rollback();
    lastTag = rec.tag;
    lastTimestamp = rec.timestamp;
    redoStack.emplace_back(std::move(rec));
    undoStack.pop_back();
  } while (hasUndo() && !lastTag.empty() && lastTag == undoStack.back().tag &&
           std::abs(static_cast<int64_t>(lastTimestamp) - undoStack.back().timestamp) < 500);
}

auto Undo::redo() -> void
{
  if (!hasRedo())
    return;
  std::string lastTag;
  Uint32 lastTimestamp;
  do
  {
    auto &rec = redoStack.back();
    rec.action();
    lastTag = rec.tag;
    lastTimestamp = rec.timestamp;
    undoStack.emplace_back(std::move(rec));
    redoStack.pop_back();
  } while (hasRedo() && !lastTag.empty() && lastTag == redoStack.back().tag &&
           std::abs(static_cast<int64_t>(lastTimestamp) - redoStack.back().timestamp) < 500);
}

auto Undo::hasUndo() const -> bool
{
  return !undoStack.empty();
}

auto Undo::hasRedo() const -> bool
{
  return !redoStack.empty();
}
