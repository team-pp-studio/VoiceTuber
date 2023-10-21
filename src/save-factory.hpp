#pragma once
#include "node.hpp"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

class SaveFactory
{
public:
  using Constructor = std::move_only_function<std::unique_ptr<Node>(std::string const & /*name*/) const>;
  template <typename T>
  auto reg(Constructor ctor) -> void
  {
    classes[T::className] = std::move(ctor);
  }
  std::unique_ptr<Node> ctor(const std::string &className, std::string const &name) const;

private:
  std::unordered_map<std::string, Constructor> classes;
};
