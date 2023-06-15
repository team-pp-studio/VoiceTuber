#pragma once
#include "uv.hpp"
#include <SDL.h>
#include <SDL_opengl.h>
#include <memory>
#include <string>

class Texture
{
public:
  Texture(uv::Uv &, std::string path, bool isUi = false);
  Texture(SDL_Surface *);
  ~Texture();
  Texture(const Texture &) = delete;
  auto ch() const -> int { return ch_; }
  auto w() const -> int { return w_; }
  auto h() const -> int { return h_; }
  auto imageData() const -> const unsigned char * { return imageData_; }
  auto texture() const -> GLuint { return texture_; }

private:
  std::string path;
  int ch_ = 4;
  int w_ = 0;
  int h_ = 0;
  unsigned char *imageData_ = nullptr;
  GLuint texture_;
  std::unique_ptr<uv::FsEvent> event;
};
