#pragma once

#include <fmt/core.h>
#include <imgui.h>

namespace ImGui
{

  decltype(auto) TextUnformatted(std::string_view text)
  {
    return ImGui::TextUnformatted(&text.begin()[0], &text.end()[0]);
  }

  // convenience alias
  decltype(auto) Text(std::string_view text)
  {
    return ImGui::TextUnformatted(text);
  }

  template <typename... Args>
  decltype(auto) TextF(fmt::format_string<Args...> format, Args &&...args)
  {
    auto const text = fmt::format(std::move(format), std::forward<Args>(args)...);
    return ImGui::TextUnformatted(text);
  }

  decltype(auto) CalcTextSize(std::string_view text, bool hide_text_after_double_hash = false, float wrap_width = -1.0f)
  {
    return ImGui::CalcTextSize(&text.begin()[0], &text.end()[0], hide_text_after_double_hash, wrap_width);
  }
} // namespace ImGui
