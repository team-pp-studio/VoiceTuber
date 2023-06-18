#pragma once
#include "wav.hpp"
#include <iostream>
#include <string>
#include <vector>

auto saveWav(std::ostream &, const Wav &wav, int sampleRate) -> void;
