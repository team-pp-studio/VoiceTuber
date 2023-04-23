#pragma once
#include "dialog.hpp"

class PreferencesDialog final : public Dialog
{
public:
  PreferencesDialog(class Preferences &, Cb);

private:
  std::reference_wrapper<Preferences> preferences;

  auto internalDraw() -> DialogState final;
};
