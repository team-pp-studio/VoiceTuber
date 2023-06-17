#include "save-wav.hpp"
#include <fstream>
#include <iostream>

namespace little_endian_io
{
  template <typename Word>
  std::ostream &writeWord(std::ostream &outs, Word value, unsigned size = sizeof(Word))
  {
    for (; size; --size, value >>= 8)
      outs.put(static_cast<char>(value & 0xFF));
    return outs;
  }
} // namespace little_endian_io
using namespace little_endian_io;

auto saveWav(std::ostream &f, const Wav &pcm, int sampleRate) -> void
{
  // Write the file headers
  f << "RIFF----WAVEfmt ";                    // (chunk size to be filled in later)
  writeWord(f, 16, 4);                        // no extension data
  writeWord(f, 1, 2);                         // PCM - integer samples
  writeWord(f, 1, 2);                         // one channel (mono file)
  writeWord(f, sampleRate, 4);                // samples per second (Hz)
  writeWord(f, (sampleRate * 16 * 1) / 8, 4); // (Sample Rate * BitsPerSample * Channels) / 8
  writeWord(f, 2, 2);  // data block size (size of two integer samples, one for each channel, in bytes)
  writeWord(f, 16, 2); // number of bits per sample (use a multiple of 8)

  // Write the data chunk header
  const auto dataChunkPos = static_cast<long>(f.tellp());
  f << "data----"; // (chunk size to be filled in later)

  for (auto a : pcm)
    writeWord(f, a, 2);

  // (We'll need the final file size to fix the chunk sizes above)
  const auto fileLength = static_cast<long>(f.tellp());

  // Fix the data chunk header to contain the data size
  f.seekp(dataChunkPos + 4);
  writeWord(f, fileLength - dataChunkPos + 8);

  // Fix the file header to contain the proper RIFF chunk size, which is (file size - 8) bytes
  f.seekp(0 + 4);
  writeWord(f, fileLength - 8, 4);
}
