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

Font::Font(std::string file, int ptsize)
  : file_(std::move(file)), ptsize_(ptsize), font([this]() {
      FontInitializer::init();
      return TTF_OpenFont(file_.c_str(), ptsize_);
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

auto Font::render(glm::vec2 pos, const std::string &txt) -> void
{
  auto &tex = getTextureFromCache(txt);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, tex.texture());
  glBegin(GL_QUADS);
  glTexCoord2f(.0f, .0f);
  glVertex2f(pos.x, pos.y + tex.h());
  glTexCoord2f(1.f, .0f);
  glVertex2f(pos.x + tex.w(), pos.y + tex.h());
  glTexCoord2f(1.f, 1.f);
  glVertex2f(pos.x + tex.w(), pos.y);
  glTexCoord2f(.0f, 1.f);
  glVertex2f(pos.x, pos.y);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);
}

auto Font::getTextureFromCache(const std::string &txt) const -> Texture &
{
  // TODO-Mika clean cache if it too big
  {
    auto it = texturesCache.find(txt);
    if (it != texturesCache.end())
      return it->second;
  }

  SDL_Surface *surface = TTF_RenderUTF8_Blended(font, txt.c_str(), {255, 255, 255, 255});
  if (!surface)
  {
    std::ostringstream ss;
    ss << "TTF_RenderText_Blended" << TTF_GetError();
    LOG(ss.str());
    throw std::runtime_error(ss.str());
  }

  auto tmp = texturesCache.emplace(txt, surface);
  assert(tmp.second);

  SDL_FreeSurface(surface);
  return tmp.first->second;
}

auto Font::getSize(const std::string &txt) const -> glm::vec2
{
  int w, h;
  TTF_SizeUTF8(font, txt.c_str(), &w, &h);
  return glm::vec2{w, h};
}

auto Font::file() const -> const std::string &
{
  return file_;
}

auto Font::ptsize() const -> int
{
  return ptsize_;
}
