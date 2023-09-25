#include "file.hpp"
#include <algorithm>
#include <cstring>
#include <cerrno>

UniqueFile open_file(std::filesystem::path const &path, char const *mode) noexcept
{
#ifdef _WIN32
  // widen the mode string to wchar_t
  wchar_t buf[32];
  std::size_t const size = std::strlen(mode);
  if (size >= std::size(buf))
  {
    errno = E2BIG;
    return nullptr;
  }
    
  *std::copy(mode, mode + size, buf) = L'\0';

  return UniqueFile(_wfopen(path.c_str(), buf));
#else
  return UniqueFile(std::fopen(path.c_str(), mode));
#endif
}
