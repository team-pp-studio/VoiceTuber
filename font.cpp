#include "font.hpp"
#include <log/log.hpp>
#include <sstream>

namespace
{
  class FontInitializer
  {
  public:
    FontInitializer()
    {
      const auto r = TTF_Init();
      if (r < 0)
      {
        std::ostringstream ss;
        ss << "TTF_Init: " << TTF_GetError();
        LOG(ss.str());
        throw std::runtime_error(ss.str());
      }
    }
    ~FontInitializer() { TTF_Quit(); }
    static auto init() -> void { static FontInitializer init; }
  };
} // namespace

Font::Font(const std::string &fileName, int size)
  : font([&]() {
      FontInitializer::init();
      return TTF_OpenFont(fileName.c_str(), size);
    }())
{
  if (!font)
    LOG("TTF_OpenFont", TTF_GetError());
}

Font::~Font()
{
  if (font)
    TTF_CloseFont(font);
}

auto Font::render(glm::vec3 pos, const std::string &txt) -> void
{
  auto &tex = getTextureFromCache(txt);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, tex.texture());
  glBegin(GL_QUADS);
  glColor4f(1.f, 1.f, 1.f, 1.f);
  glTexCoord2f(.0f, .0f);
  glVertex3f(pos.x, pos.y + tex.h(), pos.z);
  glTexCoord2f(1.f, .0f);
  glVertex3f(pos.x + tex.w(), pos.y + tex.h(), pos.z);
  glTexCoord2f(1.f, 1.f);
  glVertex3f(pos.x + tex.w(), pos.y, pos.z);
  glTexCoord2f(.0f, 1.f);
  glVertex3f(pos.x, pos.y, pos.z);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);
}

auto Font::getTextureFromCache(const std::string &txt) const -> Texture &
{
  {
    auto it = texturesCache.find(txt);
    if (it != texturesCache.end())
      return it->second;
  }

  SDL_Surface *surface = TTF_RenderText_Blended(font, txt.c_str(), {255, 255, 255, 255});
  if (!surface)
  {
    std::ostringstream ss;
    ss << "TTF_RenderText_Blended" << TTF_GetError();
    LOG(ss.str());
    throw std::runtime_error(ss.str());
  }

  LOG("Surface", surface->w, surface->h);

  auto tmp = texturesCache.emplace(txt, surface);
  assert(tmp.second);

  SDL_FreeSurface(surface);
  return tmp.first->second;
}

auto Font::getH(const std::string &txt) const -> int
{
  return getTextureFromCache(txt).h();
}
