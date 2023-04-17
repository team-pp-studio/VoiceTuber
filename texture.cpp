#include "texture.hpp"
#include <cassert>
#include <log/log.hpp>
#include <sstream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#pragma GCC diagnostic ignored "-Wextra-semi-stmt"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#include "stb/stb_image.h"
#pragma GCC diagnostic pop

Texture::Texture(const std::string &path)
  : imageData_([&]() {
      stbi_set_flip_vertically_on_load(1);
      auto ret = stbi_load(path.c_str(), &w_, &h_, &ch_, STBI_rgb_alpha);
      if (!ret)
      {
        std::ostringstream ss;
        ss << "Error loading image: " << path;
        throw std::runtime_error(ss.str());
      }
      return ret;
    }()),
    texture_([&]() {
      assert((ch_ == 4 || ch_ == 3) && "The number of channels should be 3 or 4.");

      GLuint ret;
      glGenTextures(1, &ret);
      glBindTexture(GL_TEXTURE_2D, ret);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      if (ch_ == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w_, h_, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData_);
      else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w_, h_, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData_);
      return ret;
    }())
{
}

Texture::Texture(SDL_Surface *surface)
  : ch_(4), w_(surface->w), h_(surface->h), texture_([&]() {
      GLuint texture;
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->pitch / surface->format->BytesPerPixel);
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   GL_RGBA,
                   surface->w,
                   surface->h,
                   0,
                   GL_RGBA,
                   GL_UNSIGNED_BYTE,
                   surface->pixels);
      glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
      return texture;
    }())
{
}

Texture::~Texture()
{
  glDeleteTextures(1, &texture_);
  if (imageData_)
    stbi_image_free(imageData_);
}
