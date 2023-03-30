#include "bouncer.hpp"
#include <SDL_opengl.h>
#include <log/log.hpp>

auto Bouncer::ingest(Wav v) -> void
{
  if (v.empty())
    return;
  offset = strength * v.back() / 0x8000;
}

auto Bouncer::render() -> void
{
  glTranslatef(.0f, offset, .0f);
}
