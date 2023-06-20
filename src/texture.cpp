#include "texture.hpp"
#include <cassert>
#include <log/log.hpp>
#include <sstream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#pragma GCC diagnostic ignored "-Wextra-semi-stmt"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#include <stb_image.h>
#pragma GCC diagnostic pop

Texture::Texture(uv::Uv &uv, std::string aPath, bool isUi)
  : path_(std::move(aPath)),
    imageData_([&]() {
      stbi_set_flip_vertically_on_load(!isUi ? 1 : 0);
      if (path_.find("engine:") != 0)
      {
        try
        {
          auto ret = stbi_load(path_.c_str(), &w_, &h_, &ch_, STBI_rgb_alpha);
          if (!ret)
          {
            std::ostringstream ss;
            ss << "Error loading image: " << path_;
            throw std::runtime_error(ss.str());
          }
          return ret;
        }
        catch (std::runtime_error &e)
        {
          LOG(e.what());
          auto engineImgPath = SDL_GetBasePath() + std::string{"assets/corrupted.png"};
          auto ret = stbi_load(engineImgPath.c_str(), &w_, &h_, &ch_, STBI_rgb_alpha);
          if (!ret)
          {
            std::ostringstream ss;
            ss << "Error loading image: " << engineImgPath;
            throw std::runtime_error(ss.str());
          }
          return ret;
        }
      }
      else
      {
        auto engineImgPath = SDL_GetBasePath() + std::string{"assets/"} + path_.substr(7);
        auto ret = stbi_load(engineImgPath.c_str(), &w_, &h_, &ch_, STBI_rgb_alpha);
        if (!ret)
        {
          std::ostringstream ss;
          ss << "Error loading image: " << engineImgPath;
          throw std::runtime_error(ss.str());
        }
        return ret;
      }
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
    }()),
    event(std::make_unique<uv::FsEvent>(uv.createFsEvent()))
{
  event->start(
    [this](std::string file, int /*events*/, int status) {
      if (status != 0)
        return;
      path_ = std::move(file);
      try
      {
        auto ret = stbi_load(path_.c_str(), &w_, &h_, &ch_, STBI_rgb_alpha);
        if (!ret)
        {
          std::ostringstream ss;
          ss << "Error loading image: " << path_;
          throw std::runtime_error(ss.str());
        }
        imageData_ = ret;
        glBindTexture(GL_TEXTURE_2D, texture_);
        if (ch_ == 4)
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w_, h_, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData_);
        else
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w_, h_, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData_);
      }
      catch (std::runtime_error &e)
      {
        LOG(e.what());
      }
    },
    path_,
    0);
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
  if (event)
    event->stop();
  glDeleteTextures(1, &texture_);
  if (imageData_)
    stbi_image_free(imageData_);
}

auto Texture::path() const -> std::string
{
  return path_;
}
