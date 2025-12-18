#pragma once

#include "audio_buffer.hpp"
#include <QString>
#include <memory>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

namespace core {

class AudioEncoder {
public:
  AudioEncoder();
  ~AudioEncoder();

  // Инициализация
  bool open(const QString &path, int sample_rate, int channels,
            int bitrate = 128000);

  // Кодирование куска данных
  bool encode_from_buffer(const AudioBuffer &buffer);

  // Завершение кодирования (ВАЖНО вызвать в конце)
  void close();

private:
  bool init_stream_and_codec();
  bool flush_encoder(); // Сброс остатков из FIFO и энкодера
  void cleanup();       // Очистка ресурсов

  QString path_;
  int sample_rate_ = 0;
  int channels_ = 0;
  int bitrate_ = 0;
  bool opened_ = false;

  // FFmpeg structures
  AVFormatContext *format_ctx_ = nullptr;
  AVCodecContext *codec_ctx_ = nullptr;
  AVStream *stream_ = nullptr;
  AVFrame *frame_ = nullptr;
  AVPacket *packet_ = nullptr;

  // Новые структуры для MP3
  SwrContext *swr_ctx_ = nullptr;
  AVAudioFifo *fifo_ = nullptr;
  int64_t pts_ = 0;
};

} // namespace core
