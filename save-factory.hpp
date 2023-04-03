#pragma once
#include "node.hpp"
#include <functional>
#include <memory>
#include <unordered_map>

class SaveFactory
{
public:
  using Ctor = std::function<auto(std::string name)->std::unique_ptr<Node>>;
  template <typename T>
  auto reg(Ctor ctor) -> void
  {
    classes[T::className] = std::move(ctor);
  }
  auto ctor(const std::string &className, std::string name) const -> std::unique_ptr<Node>;

private:
  std::unordered_map<std::string, Ctor> classes;
};
