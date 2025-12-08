#include "app.hpp"
#include <QDir>
#include <QFileInfo>

#include "core/audio_buffer.hpp"
#include "core/audio_buffer.hpp" // apply_reverb, ReverbParams
#include "core/audio_decoder.hpp"
#include "core/audio_encoder.hpp"
#include "log/log.hpp"

namespace app {

App::App(int &argc, char **argv) : QApplication(argc, argv) {}
App::~App() = default;

void App::load_audio_file(const QString &path) {
  TE_INFO("loading file: {}", path.toStdString());
  file_path_ = path;
}

void App::process_audio_file() {
  if (file_path_.isEmpty()) {
    TE_ERROR("No file loaded");
    return;
  }

  TE_INFO("processing file: {}", file_path_.toStdString());

  QFileInfo info(file_path_);
  const QString dir = info.absolutePath();
  const QString baseName = info.completeBaseName();

  const QString outName = baseName + "_grustnified.wav";
  const QString outPath = QDir(dir).filePath(outName);

  TE_INFO("output file: {}", outPath.toStdString());

  core::AudioDecoder decoder(file_path_);
  if (!decoder.open()) {
    TE_ERROR("Failed to open decoder for {}", file_path_.toStdString());
    return;
  }

  core::AudioBuffer buffer;
  if (!decoder.decode_to_buffer(buffer)) {
    TE_ERROR("Failed to decode audio file {}", file_path_.toStdString());
    return;
  }

  if (buffer.sample_rate <= 0 || buffer.channels <= 0 ||
      buffer.samples.empty()) {
    TE_ERROR("Decoded buffer is empty or invalid");
    return;
  }

  TE_INFO("decoded: sample_rate={} channels={} frames={}", buffer.sample_rate,
          buffer.channels, buffer.samples.size() / buffer.channels);

  core::ReverbParams rp;
  rp.mix = 0.05f;
  rp.room_size = 0.5f;
  rp.damp = 0.3f;

  core::AudioBuffer with_reverb = core::reverb(buffer, rp);

  const float speed_factor = 1.15f;
  core::AudioBuffer processed = core::change_speed(with_reverb, speed_factor);

  if (processed.samples.empty()) {
    TE_ERROR("Processed buffer is empty after reverb+slowdown");
    return;
  }

  TE_INFO("processed: sample_rate={} channels={} frames={}",
          processed.sample_rate, processed.channels,
          processed.samples.size() / processed.channels);

  core::AudioEncoder encoder(outPath, processed.sample_rate,
                             processed.channels);
  if (!encoder.open()) {
    TE_ERROR("Failed to open encoder for {}", outPath.toStdString());
    return;
  }

  if (!encoder.encode_from_buffer(processed)) {
    TE_ERROR("Failed to encode processed audio to {}", outPath.toStdString());
    return;
  }

  TE_INFO("Successfully grustnified {}", outPath.toStdString());
}

} // namespace app
