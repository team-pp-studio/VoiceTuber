#pragma once
#include "audio-level.hpp"
#include "dialog.hpp"

class PreferencesDialog final : public Dialog
{
public:
  PreferencesDialog(class Preferences &, class AudioOutput &, class AudioInput &, Cb);

private:
  std::reference_wrapper<Preferences> preferences;
  std::reference_wrapper<AudioOutput> audioOutput;
  std::reference_wrapper<AudioInput> audioInput;
  AudioLevel audioLevel;

  auto internalDraw() -> DialogState final;
  auto updateInputAudio(std::string) -> void;
  auto updateOutputAudio(std::string) -> void;
};
