#include "core/audio_buffer.hpp"
#include "core/audio_decoder.hpp"
#include "log/log.hpp"
#include <QString>
#include <gtest/gtest.h>
#include <qcontainerfwd.h>

static QString testDataFile(const char *name) {
  return QString(TEST_DATA_DIR) + "/" + name;
}

TEST(AudioDecoderTest, OpenValidFile) {
  grustnify::Log::Init();
  QString path = testDataFile("sine_440hz_44-1kHz_2sec.wav");
  core::AudioDecoder decoder(path);

  EXPECT_TRUE(decoder.open());

  core::AudioBuffer buffer;
  EXPECT_TRUE(decoder.decode_to_buffer(buffer));

  EXPECT_GT(buffer.sample_rate, 0);
  EXPECT_GE(buffer.channels, 1);
  EXPECT_FALSE(buffer.samples.empty());

  double seconds = static_cast<double>(buffer.samples.size()) /
                   (buffer.sample_rate * buffer.channels);

  EXPECT_GT(seconds, 0.5);
  EXPECT_LT(seconds, 3.0);
}

TEST(AudioDecoderTest, OpenNonExistingFileFails) {
  QString path = QStringLiteral("does_not_exist_12345.wav");
  core::AudioDecoder decoder(path);

  EXPECT_FALSE(decoder.open());
}

TEST(AudioDecoderTest, DecodeSineFrequency) {
  QString path = testDataFile("sine_440hz_44-1kHz_2sec.wav");
  core::AudioDecoder decoder(path);

  ASSERT_TRUE(decoder.open());

  core::AudioBuffer buffer;
  ASSERT_TRUE(decoder.decode_to_buffer(buffer));

  ASSERT_GT(buffer.sample_rate, 0);
  ASSERT_EQ(buffer.channels, 1); // тестовый файл должен быть mono
  ASSERT_FALSE(buffer.samples.empty());

  const int sr = buffer.sample_rate;
  const double duration = static_cast<double>(buffer.samples.size()) / sr;

  EXPECT_NEAR(duration, 2.0, 0.05); // около 2 секунд

  //
  // 1) Проверяем частоту синусоиды через нулевые пересечения.
  //

  const std::vector<float> &s = buffer.samples;
  int zero_crosses = 0;

  for (size_t i = 1; i < s.size(); ++i) {
    if ((s[i - 1] <= 0 && s[i] > 0) || (s[i - 1] >= 0 && s[i] < 0)) {
      zero_crosses++;
    }
  }

  // Для синусоиды: два нулевых пересечения = один период.
  double estimated_freq = (zero_crosses / 2.0) / duration;

  EXPECT_NEAR(estimated_freq, 440.0, 5.0); // 440 Гц с допуском +-5 Гц

  //
  // 2) Проверяем амплитуду (должна быть примерно 1.0 или то, что в тестовом
  // файле)
  //

  float max_amp = 0.0f;
  for (float v : s) {
    if (std::abs(v) > max_amp)
      max_amp = std::abs(v);
  }

  EXPECT_GT(max_amp, 0.7f);
  EXPECT_LT(max_amp, 1.1f);
}
