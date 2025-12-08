#pragma once
#include <vector>
namespace core {
struct AudioBuffer {
  int sample_rate;
  int channels;
  std::vector<float> samples;
};

struct ReverbParams {
  float mix = 0.3f;
  float room_size = 0.8f;
  float damp = 0.3f;
};
AudioBuffer change_speed(const core::AudioBuffer &buffer, float speed_factor);
AudioBuffer reverb(const core::AudioBuffer &buffer, const ReverbParams &p);
} // namespace core
