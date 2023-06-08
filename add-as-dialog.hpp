#pragma once

#include "dialog.hpp"

class AddAsDialog final : public Dialog
{
public:
  enum class NodeType { sprite, mouth, eye, aiMouth };
  using Cb = std::function<auto(bool, NodeType)->void>;
  AddAsDialog(std::string path, Cb cb);

private:
  std::string path;
  NodeType nodeType = NodeType::sprite;
  auto internalDraw() -> DialogState final;
};
