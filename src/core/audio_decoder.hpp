#pragma once
#include <QString>
#include <core/audio_buffer.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}

namespace core {
class AudioDecoder {
public:
  AudioDecoder(QString &input_path);
  ~AudioDecoder();
  bool open();
  bool decode_to_buffer(core::AudioBuffer &buffer);

private:
  QString path_ = "";
  AVFormatContext *format_ctx_ = nullptr;
  AVCodecContext *codec_ctx_ = nullptr;
  int audio_stream_index_ = -1;
  AVChannelLayout input_channel_layout_;
  AVChannelLayout output_channel_layout_;

  int input_sample_rate_ = 0;
  int output_sample_rate_ = 0;
  int output_channels_ = 0;
};
} // namespace core
