#pragma once

#include "dialog.hpp"
#include <filesystem>
#include <functional>

class AddAsDialog final : public Dialog
{
public:
  enum class NodeType {
    sprite,
    mouth,
    eye,
    aiMouth
  };

  using Callback = std::move_only_function<void(bool, NodeType)>;

  AddAsDialog(std::filesystem::path path, Callback cb);

private:
  std::filesystem::path path;
  NodeType nodeType = NodeType::sprite;
  auto internalDraw() -> DialogState final;
};
