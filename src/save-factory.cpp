#include "save-factory.hpp"

std::unique_ptr<Node> SaveFactory::ctor(const std::string &className, std::string const &name) const
{
  auto it = classes.find(className);
  if (it == std::end(classes))
    return nullptr;
  return it->second(std::move(name));
}
