#include "wav-2-visemes.hpp"
#include <pocketsphinx.h>

Wav2Visemes::Wav2Visemes(Cb cb) : cb(std::move(cb)) {}
auto Wav2Visemes::ingest(Wav) -> void {}
