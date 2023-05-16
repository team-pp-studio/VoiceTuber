#pragma once
#include <imgui/imgui.h>
#include <string>
#include <utility>

#define DECLARE_IMGUI_RAII(X)                       \
  class X                                           \
  {                                                 \
  public:                                           \
    template <typename... Args>                     \
    X(Args &&...args)                               \
    {                                               \
      ImGui::Begin##X(std::forward<Args>(args)...); \
    }                                               \
    X(X &&x) { x.isEmpty = true; }                  \
    X &operator=(X &&x)                             \
    {                                               \
      x.isEmpty = true;                             \
      return *this;                                 \
    }                                               \
    ~X()                                            \
    {                                               \
      if (!isEmpty)                                 \
        ImGui::End##X();                            \
    }                                               \
                                                    \
  private:                                          \
    bool isEmpty = false;                           \
  }

#define DECLARE_IMGUI_RAII_BOOL(X)                                        \
  class X                                                                 \
  {                                                                       \
  public:                                                                 \
    template <typename... Args>                                           \
    X(Args &&...args) : res(ImGui::Begin##X(std::forward<Args>(args)...)) \
    {                                                                     \
    }                                                                     \
    X(X &&x) : res(x.res) { x.isEmpty = true; }                           \
    X &operator=(X &&x)                                                   \
    {                                                                     \
      res = x.res;                                                        \
      x.isEmpty = true;                                                   \
      return *this;                                                       \
    }                                                                     \
    ~X()                                                                  \
    {                                                                     \
      if (!isEmpty && res)                                                \
        ImGui::End##X();                                                  \
    }                                                                     \
    operator bool() const { return res; }                                 \
                                                                          \
  private:                                                                \
    bool res;                                                             \
    bool isEmpty = false;                                                 \
  }

class Texture;

namespace Ui
{
  auto textRj(const std::string &, float offset = 0.f) -> void;
  DECLARE_IMGUI_RAII(Disabled);
  DECLARE_IMGUI_RAII_BOOL(ListBox);
  DECLARE_IMGUI_RAII_BOOL(Menu);
  DECLARE_IMGUI_RAII_BOOL(MainMenuBar);
  DECLARE_IMGUI_RAII_BOOL(Table);

  class Window
  {
  public:
    template <typename... Args>
    Window(Args &&...args) : res(ImGui::Begin(std::forward<Args>(args)...))
    {
    }
    Window(Window &&x) { x.isEmpty = true; }
    Window &operator=(Window &&x)
    {
      x.isEmpty = true;
      return *this;
    }
    ~Window() { ImGui::End(); }
    operator bool() const { return res; }

  private:
    bool res;
    bool isEmpty = false;
  };

  auto BtnImg(const std::string &id, const Texture &, float w, float h) -> bool;
} // namespace Ui
