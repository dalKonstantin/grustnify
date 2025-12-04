#include <vector>
namespace core {
struct AudioBuffer {
  int sample_rate;
  int channels;
  std::vector<float> samples;
};
} // namespace core
