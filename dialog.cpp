#include "dialog.hpp"
#include <imgui/imgui.h>

Dialog::Dialog(Cb cb) : cb(std::move(cb))
{
  ImGui::OpenPopup("modal");
}
