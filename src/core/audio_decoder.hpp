#pragma once
#include <QString>
#include <core/audio_buffer.hpp>
#include <cstdint>
#include <libavcodec/packet.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}

namespace core {
class AudioDecoder {
public:
  AudioDecoder();
  ~AudioDecoder();

  bool open(const QString &input_path);
  int decode(uint8_t *output_buffer, int output_buffer_size);
  void close();

  int sample_rate() const;
  int channels() const;
  int bytes_per_samples() const;
  int birate() const;
  int64_t duration_us() const;
  QString sample_format_name() const;

private:
  bool init_resample();

  QString input_path_;
  AVFormatContext *format_context_ = nullptr;
  AVCodecContext *codec_context_ = nullptr;
  SwrContext *swr_context_ = nullptr;
  AVPacket *packet_ = nullptr;
  AVFrame *frame_ = nullptr;

  int stream_index_ = -1;
  AVChannelLayout output_channel_layout_{};
  AVSampleFormat output_sample_format_ = AV_SAMPLE_FMT_FLT;
  int output_sample_rate_ = 0;

  bool is_open_ = false;

  bool init_codec_and_resampler();
  bool read_all_frames_to_buffer(AudioBuffer &out_buffer);
};
} // namespace core
