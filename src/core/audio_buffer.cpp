#include "core/audio_buffer.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>
namespace core {

AudioBuffer change_speed(const AudioBuffer &in, float speed_factor) {
  // speed_factor > 1.0 => медленнее и ниже тон
  // speed_factor < 1.0 => быстрее и выше тон

  AudioBuffer out;
  out.sample_rate = in.sample_rate;
  out.channels = in.channels;

  if (speed_factor <= 0.0f || in.channels <= 0 || in.samples.empty()) {
    return out;
  }

  const int channels = in.channels;
  const std::size_t in_frames = in.samples.size() / channels;
  const std::size_t out_frames =
      static_cast<std::size_t>(in_frames * speed_factor);

  out.samples.resize(out_frames * channels);

  for (std::size_t n = 0; n < out_frames; ++n) {
    float in_pos = static_cast<float>(n) / speed_factor;

    std::size_t i0 = static_cast<std::size_t>(in_pos);
    float frac = in_pos - static_cast<float>(i0);

    if (i0 >= in_frames - 1) {
      i0 = in_frames - 1;
      frac = 0.0f;
    }

    std::size_t i1 = (i0 + 1 < in_frames) ? (i0 + 1) : i0;

    for (int ch = 0; ch < channels; ++ch) {
      float s0 = in.samples[i0 * channels + ch];
      float s1 = in.samples[i1 * channels + ch];
      float s = s0 + (s1 - s0) * frac;

      out.samples[n * channels + ch] = s;
    }
  }

  return out;
}

AudioBuffer reverb(const AudioBuffer &in, const ReverbParams &p) {
  AudioBuffer out;
  out.sample_rate = in.sample_rate;
  out.channels = in.channels;

  if (in.sample_rate <= 0 || in.channels <= 0 || in.samples.empty()) {
    return out;
  }

  const int sr = in.sample_rate;
  const int channels = in.channels;
  const size_t frames = in.samples.size() / channels;

  out.samples.resize(in.samples.size());

  float mix = std::clamp(p.mix, 0.0f, 1.0f);
  float room_size = std::clamp(p.room_size, 0.0f, 1.0f);
  float damp = std::clamp(p.damp, 0.0f, 1.0f);

  const float dry = 1.0f - mix;
  const float wet = mix;

  constexpr int NUM_COMBS = 4;
  constexpr int NUM_ALLPASSES = 2;

  const float comb_delays_ms[NUM_COMBS] = {29.7f, 37.1f, 41.1f, 43.7f};
  const float allpass_delays_ms[NUM_ALLPASSES] = {5.0f, 1.7f};

  const float base_feedback = 0.75f;
  const float feedback = base_feedback + room_size * 0.2f; // ~0.75..0.95

  const float allpass_gain = 0.5f;

  std::vector<float> in_ch(frames);
  std::vector<float> out_ch(frames);

  for (int ch = 0; ch < channels; ++ch) {

    for (size_t n = 0; n < frames; ++n) {
      in_ch[n] = in.samples[n * channels + ch];
    }

    std::array<std::vector<float>, NUM_COMBS> comb_buffers;
    std::array<size_t, NUM_COMBS> comb_indices{};

    for (int i = 0; i < NUM_COMBS; ++i) {
      int delay_samples = static_cast<int>(comb_delays_ms[i] * 0.001f * sr);
      delay_samples = std::max(delay_samples, 1);
      comb_buffers[i].assign(delay_samples, 0.0f);
      comb_indices[i] = 0;
    }

    std::array<std::vector<float>, NUM_ALLPASSES> allpass_buffers;
    std::array<size_t, NUM_ALLPASSES> allpass_indices{};

    for (int i = 0; i < NUM_ALLPASSES; ++i) {
      int delay_samples = static_cast<int>(allpass_delays_ms[i] * 0.001f * sr);
      delay_samples = std::max(delay_samples, 1);
      allpass_buffers[i].assign(delay_samples, 0.0f);
      allpass_indices[i] = 0;
    }

    for (size_t n = 0; n < frames; ++n) {
      float x = in_ch[n];

      float comb_sum = 0.0f;
      for (int i = 0; i < NUM_COMBS; ++i) {
        auto &buf = comb_buffers[i];
        auto &idx = comb_indices[i];

        float y = buf[idx];
        buf[idx] = x + y * feedback * (1.0f - damp);

        idx++;
        if (idx >= buf.size())
          idx = 0;

        comb_sum += y;
      }

      float ap = comb_sum;
      for (int i = 0; i < NUM_ALLPASSES; ++i) {
        auto &buf = allpass_buffers[i];
        auto &idx = allpass_indices[i];

        float buf_y = buf[idx];
        float v = ap - buf_y;
        buf[idx] = ap + buf_y * allpass_gain;

        idx++;
        if (idx >= buf.size())
          idx = 0;

        ap = v;
      }

      float wet_sample = ap;
      float dry_sample = x;

      out_ch[n] = dry * dry_sample + wet * wet_sample;
    }

    for (size_t n = 0; n < frames; ++n) {
      out.samples[n * channels + ch] = out_ch[n];
    }
  }

  return out;
}

} // namespace core
