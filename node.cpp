#include "node.hpp"
#include <SDL_opengl.h>

auto Node::renderAll() -> void
{
  glPushMatrix();
  // Apply transformations
  glTranslatef(loc.x, loc.y, 0.0f);       // Move the sprite
  glTranslatef(pivot.x, pivot.y, 0.0f);   // Move the pivot point
  glRotatef(rot, 0.0f, 0.0f, 1.0f);       // Rotate the sprite
  glScalef(scale.x, scale.y, 1.0f);       // Scale the sprite
  glTranslatef(-pivot.x, -pivot.y, 0.0f); // Move the pivot point back

  render();
  for (auto n : nodes)
    n.get().renderAll();
  glPopMatrix();
}
