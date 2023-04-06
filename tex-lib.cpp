#include "tex-lib.hpp"
#include <cassert>

auto TexLib::queryTex(const std::string &v) -> const Texture &
{
  auto it = lib.find(v);
  if (it != std::end(lib))
    return it->second;
  auto tmp = lib.emplace(v, v);
  assert(tmp.second);
  return tmp.first->second;
}
