#include "mouth.hpp"
#include "ui.hpp"
#include "undo.hpp"
#include "wav-2-visemes.hpp"
#include <imgui/imgui.h>

Mouth::Mouth(Wav2Visemes &wav2Visemes, Lib &lib, Undo &aUndo, const std::filesystem::path &path)
  : Node(lib, aUndo, [&path]() { return path.filename().string(); }()), sprite(lib, aUndo, path), wav2Visemes(wav2Visemes)
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

Mouth::~Mouth()
{
  wav2Visemes.get().unreg(*this);
}

auto Mouth::render(float dt, Node *hovered, Node *selected) -> void
{
  sprite.frame(viseme2Sprite[viseme] % sprite.numFrames());
  sprite.render();
  Node::render(dt, hovered, selected);
}

auto Mouth::renderUi() -> void
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
        [&f, newF = f, alive = std::weak_ptr<int>(alive), this, vis]() {
          if (!alive.lock())
            return;
          f = newF;
          viseme = vis;
          using namespace std::chrono_literals;
          freezeTime = std::chrono::high_resolution_clock::now() + 1s;
        },
        [&f, oldF, alive = std::weak_ptr<int>(alive), this, vis]() {
          if (!alive.lock())
            return;
          f = oldF;
          viseme = vis;
          using namespace std::chrono_literals;
          freezeTime = std::chrono::high_resolution_clock::now() + 1s;
        });
    }
    ImGui::SameLine();

    char btnTitle[16];
    sprintf(btnTitle, "Test##%s", txt);

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

auto Mouth::ingest(Viseme v) -> void
{
  if (std::chrono::high_resolution_clock::now() < freezeTime)
    return;
  viseme = v;
}

auto Mouth::save(OStrm &strm) const -> void
{
  ::ser(strm, className);
  ::ser(strm, name);
  ::ser(strm, *this);
  sprite.save(strm);
  Node::save(strm);
}

auto Mouth::load(IStrm &strm) -> void
{
  ::deser(strm, *this);
  sprite.load(strm);
  Node::load(strm);
}

auto Mouth::h() const -> float
{
  return sprite.h();
}

auto Mouth::isTransparent(glm::vec2 v) const -> bool
{
  return sprite.isTransparent(v);
}

auto Mouth::w() const -> float
{
  return sprite.w();
}
