#pragma once

#include <cstdio>
#include <filesystem>
#include <memory>

struct FileDeleter
{
  void operator()(std::FILE *fp) const noexcept { std::fclose(fp); }
};

using UniqueFile = std::unique_ptr<std::FILE, FileDeleter>;

// just like fopen, return null on error setting errno
UniqueFile open_file(std::filesystem::path const &path, char const *mode) noexcept;
