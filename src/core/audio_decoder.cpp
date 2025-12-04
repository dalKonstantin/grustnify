#include "core/audio_decoder.hpp"
#include "log/log.hpp"
#include <QString>
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>

namespace core {
AudioDecoder::AudioDecoder(QString &input_path) : path_(input_path) {}
AudioDecoder::~AudioDecoder() {};
bool AudioDecoder::open() {

  // Step 1: Open the input file and create format context
  if (avformat_open_input(&format_ctx_, path_.toUtf8().constData(), nullptr,
                          nullptr) != 0) {
    TE_ERROR("Could not open input file.");
    return false;
  }
  // Step 2: Retrieve Stream information;
  if (avformat_find_stream_info(format_ctx_, nullptr) < 0) {

    TE_ERROR("Could not find stream information.");
    return false;
  }
  // Step 3: Find the audio stream
  audio_stream_index_ =
      av_find_best_stream(format_ctx_, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
  if (audio_stream_index_ < 0) {
    TE_TRACE("Could not find audio stream");
    return false;
  }

  AVStream *stream = format_ctx_->streams[audio_stream_index_];
  // Step 4: Get codec for audio stream
  const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
  if (!codec) {
    TE_ERROR("Could not find decoder.");
    return false;
  }

  codec_ctx_ = avcodec_alloc_context3(codec);
  if (!codec_ctx_ ||
      avcodec_parameters_to_context(codec_ctx_, stream->codecpar) < 0) {
    TE_ERROR("Could not copy codec params");
    return false;
  }

  if (avcodec_open2(codec_ctx_, codec, nullptr) < 0) {
    TE_ERROR("Could not open codec.");
    return false;
  }

  if (codec_ctx_->ch_layout.order == AV_CHANNEL_ORDER_UNSPEC) {
    av_channel_layout_default(&codec_ctx_->ch_layout,
                              codec_ctx_->ch_layout.nb_channels);
  }
  av_channel_layout_copy(&input_channel_layout_, &codec_ctx_->ch_layout);

  output_sample_rate_ = codec_ctx_->sample_rate;
  output_channels_ = codec_ctx_->ch_layout.nb_channels;
  av_channel_layout_default(&output_channel_layout_, output_channels_);

  // Init resampler

  return true;
};
bool decode_to_buffer(core::AudioBuffer &buffer) {};
} // namespace core
