#pragma once
#include "audio-level.hpp"
#include "dialog.hpp"

class PreferencesDialog final : public Dialog
{
public:
  PreferencesDialog(class Preferences &, class AudioOut &, class AudioIn &, Callback);

private:
  std::reference_wrapper<Preferences> preferences;
  std::reference_wrapper<AudioOut> audioOut;
  std::reference_wrapper<AudioIn> audioIn;
  AudioLevel audioLevel;

  auto internalDraw() -> DialogState final;
  auto updateAudioIn(std::string) -> void;
  auto updateAudioOut(std::string) -> void;
};
