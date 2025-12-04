#include <vector>
namespace core {
struct AudioBuffer {
  int sample_rate;
  int channels;
  std::vector<float> samples;
};

core::AudioBuffer Slow(core::AudioBuffer &buffer);
core::AudioBuffer Reverb(core::AudioBuffer &buffer);
} // namespace core
