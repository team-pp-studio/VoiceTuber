#include "mouth.hpp"
#include "image-list.hpp"
#include "sprite-sheet.hpp"
#include "ui.hpp"
#include "undo.hpp"
#include "wav-2-visemes.hpp"
#include <fmt/core.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

template <typename S, typename ClassName>
Mouth<S, ClassName>::Mouth(Wav2Visemes &wav2Visemes,
                           Lib &lib,
                           Undo &aUndo,
                           const std::filesystem::path &path)
  : Node(lib, aUndo, [&path]() { return path.filename().string(); }()),
    sprite(lib, aUndo, path),
    wav2Visemes(wav2Visemes)
{
  viseme2Sprite[Viseme::sil] = 0;
  viseme2Sprite[Viseme::PP] = 1;
  viseme2Sprite[Viseme::FF] = 2;
  viseme2Sprite[Viseme::TH] = 3;
  viseme2Sprite[Viseme::DD] = 4;
  viseme2Sprite[Viseme::kk] = 5;
  viseme2Sprite[Viseme::CH] = 6;
  viseme2Sprite[Viseme::SS] = 7;
  viseme2Sprite[Viseme::nn] = 8;
  viseme2Sprite[Viseme::RR] = 9;
  viseme2Sprite[Viseme::aa] = 10;
  viseme2Sprite[Viseme::E] = 11;
  viseme2Sprite[Viseme::I] = 12;
  viseme2Sprite[Viseme::O] = 13;
  viseme2Sprite[Viseme::U] = 14;
  wav2Visemes.reg(*this);
}

template <typename S, typename ClassName>
Mouth<S, ClassName>::~Mouth()
{
  wav2Visemes.get().unreg(*this);
}

template <typename S, typename ClassName>
auto Mouth<S, ClassName>::do_clone() const -> std::shared_ptr<Node>
{
  return std::make_shared<Mouth>(*this);
}

template <typename S, typename ClassName>
auto Mouth<S, ClassName>::render(float dt, Node *hovered, Node *selected) -> void
{
  if (sprite.numFrames() > 0)
    sprite.frame(viseme2Sprite[viseme] % sprite.numFrames());
  sprite.render();
  Node::render(dt, hovered, selected);
}

template <typename S, typename ClassName>
auto Mouth<S, ClassName>::renderUi() -> void
{
  Node::renderUi();
  sprite.renderUi();
  ImGui::TableNextColumn();

  {
    ImGui::PushStyleColor(
      ImGuiCol_Text,
      ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]); // Set text color to disabled color
    Ui::textRj("Viseme");
    ImGui::TableNextColumn();
    char str[16];
    switch (viseme)
    {
    case Viseme::sil: strcpy(str, "sil"); break;
    case Viseme::PP: strcpy(str, "PP"); break;
    case Viseme::FF: strcpy(str, "FF"); break;
    case Viseme::TH: strcpy(str, "TH"); break;
    case Viseme::DD: strcpy(str, "DD"); break;
    case Viseme::kk: strcpy(str, "kk"); break;
    case Viseme::CH: strcpy(str, "CH"); break;
    case Viseme::SS: strcpy(str, "SS"); break;
    case Viseme::nn: strcpy(str, "nn"); break;
    case Viseme::RR: strcpy(str, "RR"); break;
    case Viseme::aa: strcpy(str, "aa"); break;
    case Viseme::E: strcpy(str, "E"); break;
    case Viseme::I: strcpy(str, "I"); break;
    case Viseme::O: strcpy(str, "O"); break;
    case Viseme::U: strcpy(str, "U"); break;
    }
    ImGui::InputText("##Viseme", str, ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor(); // Restore the original text color
  }

  auto visUi = [&](auto vis, auto txt, auto txt2) {
    auto &f = viseme2Sprite[vis];
    ImGui::TableNextColumn();
    Ui::textRj(txt);
    ImGui::TableNextColumn();
    const auto oldF = f;
    if (ImGui::InputInt(txt2, &f))
    {
      undo.get().record(
        [&f, newF = f, alive = weak_self(), vis]() {
          if (auto self = alive.lock())
          {
            using namespace std::chrono_literals;
            f = std::move(newF);
            self->viseme = std::move(vis);
            self->freezeTime = std::chrono::high_resolution_clock::now() + 1s;
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
            return;
          }
        },
        [&f, oldF, alive = weak_self(), vis]() {
          if (auto self = alive.lock())
          {
            using namespace std::chrono_literals;
            f = std::move(oldF);
            self->viseme = std::move(vis);
            self->freezeTime = std::chrono::high_resolution_clock::now() + 1s;
          }
          else
          {
            SPDLOG_INFO("this was destroyed");
            return;
          }
        });
    }
    ImGui::SameLine();

    char btnTitle[16];
    *fmt::format_to_n(btnTitle, std::size(btnTitle) - 1, "Test##{}", txt).out = 0;

    if (ImGui::Button(btnTitle))
    {
      viseme = vis;
      using namespace std::chrono_literals;
      freezeTime = std::chrono::high_resolution_clock::now() + 1s;
    }
  };
  visUi(Viseme::sil, "sil", "##sil");
  visUi(Viseme::PP, "PP", "##PP");
  visUi(Viseme::FF, "FF", "##FF");
  visUi(Viseme::TH, "TH", "##TH");
  visUi(Viseme::DD, "DD", "##DD");
  visUi(Viseme::kk, "kk", "##kk");
  visUi(Viseme::CH, "CH", "##CH");
  visUi(Viseme::SS, "SS", "##SS");
  visUi(Viseme::nn, "nn", "##nn");
  visUi(Viseme::RR, "RR", "##RR");
  visUi(Viseme::aa, "aa", "##aa");
  visUi(Viseme::E, "E", "##E");
  visUi(Viseme::I, "I", "##I");
  visUi(Viseme::O, "O", "##O");
  visUi(Viseme::U, "U", "##U");
}

template <typename S, typename ClassName>
auto Mouth<S, ClassName>::ingest(Viseme v) -> void
{
  if (std::chrono::high_resolution_clock::now() < freezeTime)
    return;
  viseme = v;
}

template <typename S, typename ClassName>
auto Mouth<S, ClassName>::save(OStrm &strm) const -> void
{
  ::ser(strm, className);
  ::ser(strm, name);
  ::ser(strm, *this);
  sprite.save(strm);
  Node::save(strm);
}

template <typename S, typename ClassName>
auto Mouth<S, ClassName>::load(IStrm &strm) -> void
{
  ::deser(strm, *this);
  sprite.load(strm);
  Node::load(strm);
}

template <typename S, typename ClassName>
auto Mouth<S, ClassName>::h() const -> float
{
  return sprite.h();
}

template <typename S, typename ClassName>
auto Mouth<S, ClassName>::isTransparent(glm::vec2 v) const -> bool
{
  return sprite.isTransparent(v);
}

template <typename S, typename ClassName>
auto Mouth<S, ClassName>::w() const -> float
{
  return sprite.w();
}

template class Mouth<SpriteSheet, SpriteSheetMouthClassName>;
template class Mouth<ImageList, ImageListMouthClassName>;
