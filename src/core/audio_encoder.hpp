#pragma once

#include "core/audio_buffer.hpp"
#include <QString>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/frame.h>
}

namespace core {

class AudioEncoder {
public:
  AudioEncoder(const QString &output_path, int sample_rate, int channels);
  ~AudioEncoder();

  bool open(); // подготовка энкодера (WAV, PCM F32)
  bool encode_from_buffer(const AudioBuffer &); // записать весь буфер
  void close();                                 // явное закрытие (опционально)

private:
  bool init_stream_and_codec();

private:
  QString path_;
  int sample_rate_ = 0;
  int channels_ = 0;

  AVFormatContext *format_ctx_ = nullptr;
  AVCodecContext *codec_ctx_ = nullptr;
  AVStream *stream_ = nullptr;
  AVFrame *frame_ = nullptr;
  AVPacket *packet_ = nullptr;
  AVChannelLayout ch_layout_{};
  bool opened_ = false;
};

} // namespace core
