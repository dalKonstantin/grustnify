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
