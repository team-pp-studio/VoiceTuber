#include "save-factory.hpp"

auto SaveFactory::ctor(const std::string &className, std::string name) const
  -> std::unique_ptr<class Node>
{
  auto it = classes.find(className);
  if (it == std::end(classes))
    return nullptr;
  return it->second(std::move(name));
}
