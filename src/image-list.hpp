#pragma once
#include "dialog.hpp"
#include "lib.hpp"
#include "node.hpp"
#include "undo.hpp"
#include <filesystem>

class ImageList : public std::enable_shared_from_this<ImageList>
{
public:
#define SER_PROP_LIST SER_PROP(texturesForSaveLoad);
  SER_DEF_PROPS()
#undef SER_PROP_LIST

  ImageList(Lib &, Undo &, const std::filesystem::path &path);
  auto frame() const -> int;
  auto frame(int) -> void;
  auto h() const -> float;
  auto isTransparent(glm::vec2) const -> bool;
  auto load(IStrm &) -> void;
  auto numFrames() const -> int;
  auto render() -> void;
  auto renderUi() -> void;
  auto save(OStrm &) const -> void;
  auto w() const -> float;

private:
  std::reference_wrapper<Lib> lib;
  std::reference_wrapper<Undo> undo;
  int frame_ = 0;
  mutable std::vector<std::string> texturesForSaveLoad;
  std::vector<std::shared_ptr<const Texture>> textures;
  std::unique_ptr<Dialog> dialog = nullptr;
};
