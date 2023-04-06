#include "texture.hpp"
#include "stb/stb_image.h"
#include <cassert>
#include <log/log.hpp>
#include <sstream>

Texture::Texture(const std::string &path)
  : texture_([&]() {
      stbi_set_flip_vertically_on_load(1);
      imageData_ = stbi_load(path.c_str(), &w_, &h_, &ch_, STBI_rgb_alpha);
      if (imageData_ == nullptr)
      {
        std::ostringstream ss;
        ss << "Error loading image: " << path;
        throw std::runtime_error(ss.str());
      }
      LOG("Number of channels:", ch_);
      assert((ch_ == 4 || ch_ == 3) && "The number of channels should be 3 or 4.");

      GLuint ret;
      glGenTextures(1, &ret);
      glGenTextures(1, &texture_);
      glBindTexture(GL_TEXTURE_2D, texture_);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      if (ch_ == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w_, h_, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData_);
      else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w_, h_, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData_);
      return texture_;
    }())
{
}

Texture::~Texture()
{
  glDeleteTextures(1, &texture_);
  stbi_image_free(imageData_);
}
