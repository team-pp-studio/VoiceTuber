#pragma once
#include <SDL_opengl.h>
#include <string>

class Texture
{
public:
  Texture(const std::string &path);
  ~Texture();
  Texture(const Texture &) = delete;
  auto ch() const -> int { return ch_; }
  auto w() const -> int { return w_; }
  auto h() const -> int { return h_; }
  auto imageData() const -> const unsigned char * { return imageData_; }
  auto texture() const -> GLuint { return texture_; }

private:
  int ch_;
  int w_;
  int h_;
  unsigned char *imageData_;
  GLuint texture_;
};
