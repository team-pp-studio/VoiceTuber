#include "tex-lib.hpp"
#include <cassert>

auto TexLib::queryTex(const std::string &v) -> std::shared_ptr<const Texture>
{
  auto it = lib.find(v);
  if (it != std::end(lib))
  {
    auto shared = it->second.lock();
    if (shared)
      return shared;
    lib.erase(it);
  }
  auto shared = std::make_shared<Texture>(v);
  auto tmp = lib.emplace(v, shared);
  assert(tmp.second);
  return shared;
}
