#include "core/audio_encoder.hpp"
#include "log/log.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/frame.h>
}

namespace core {

AudioEncoder::AudioEncoder(const QString &output_path, int sample_rate,
                           int channels)
    : path_(output_path), sample_rate_(sample_rate), channels_(channels) {}

AudioEncoder::~AudioEncoder() { close(); }

void AudioEncoder::close() {
  if (!opened_) {
    // даже если не открыт, всё равно подчистим
  }

  if (frame_) {
    av_frame_free(&frame_);
    frame_ = nullptr;
  }
  if (packet_) {
    av_packet_free(&packet_);
    packet_ = nullptr;
  }
  if (codec_ctx_) {
    avcodec_free_context(&codec_ctx_);
    codec_ctx_ = nullptr;
  }

  if (format_ctx_) {
    if (opened_) {
      av_write_trailer(format_ctx_);
    }
    if (!(format_ctx_->oformat->flags & AVFMT_NOFILE) && format_ctx_->pb) {
      avio_closep(&format_ctx_->pb);
    }
    avformat_free_context(format_ctx_);
    format_ctx_ = nullptr;
  }

  av_channel_layout_uninit(&ch_layout_);

  opened_ = false;
}

bool AudioEncoder::init_stream_and_codec() {
  // Контекст формата (контейнера) уже создан в open()
  AVCodecID codec_id = AV_CODEC_ID_PCM_F32LE; // WAV с float32

  const AVCodec *codec = avcodec_find_encoder(codec_id);
  if (!codec) {
    TE_ERROR("AudioEncoder: Could not find encoder for PCM_F32LE");
    return false;
  }

  stream_ = avformat_new_stream(format_ctx_, codec);
  if (!stream_) {
    TE_ERROR("AudioEncoder: Could not create new stream");
    return false;
  }

  codec_ctx_ = avcodec_alloc_context3(codec);
  if (!codec_ctx_) {
    TE_ERROR("AudioEncoder: Could not allocate codec context");
    return false;
  }

  codec_ctx_->codec_id = codec_id;
  codec_ctx_->codec_type = AVMEDIA_TYPE_AUDIO;
  codec_ctx_->sample_rate = sample_rate_;
  codec_ctx_->sample_fmt = AV_SAMPLE_FMT_FLT; // тот же float, что в AudioBuffer

  av_channel_layout_default(&ch_layout_, channels_);
  codec_ctx_->ch_layout = ch_layout_;

  codec_ctx_->time_base = AVRational{1, sample_rate_};

  if (format_ctx_->oformat->flags & AVFMT_GLOBALHEADER) {
    codec_ctx_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }

  if (avcodec_open2(codec_ctx_, codec, nullptr) < 0) {
    TE_ERROR("AudioEncoder: Could not open codec");
    return false;
  }

  if (avcodec_parameters_from_context(stream_->codecpar, codec_ctx_) < 0) {
    TE_ERROR("AudioEncoder: Could not copy codec parameters to stream");
    return false;
  }

  stream_->time_base = codec_ctx_->time_base;

  // Выделяем frame и packet
  frame_ = av_frame_alloc();
  packet_ = av_packet_alloc();
  if (!frame_ || !packet_) {
    TE_ERROR("AudioEncoder: Could not allocate frame/packet");
    return false;
  }

  frame_->format = codec_ctx_->sample_fmt;
  frame_->sample_rate = codec_ctx_->sample_rate;
  av_channel_layout_copy(&frame_->ch_layout, &codec_ctx_->ch_layout);

  // frame_size может быть 0 для PCM; тогда мы выберем блок сами
  if (codec_ctx_->frame_size > 0) {
    frame_->nb_samples = codec_ctx_->frame_size;
  } else {
    frame_->nb_samples = 1024;
  }

  if (av_frame_get_buffer(frame_, 0) < 0) {
    TE_ERROR("AudioEncoder: Could not allocate frame buffer");
    return false;
  }

  return true;
}

bool AudioEncoder::open() {
  close(); // очистка на всякий случай

  // Создаём выходной AVFormatContext для WAV
  AVFormatContext *fmt = nullptr;
  if (avformat_alloc_output_context2(&fmt, nullptr, nullptr,
                                     path_.toUtf8().constData()) < 0 ||
      !fmt) {
    TE_ERROR("AudioEncoder: Could not allocate output context");
    return false;
  }

  format_ctx_ = fmt;

  if (!init_stream_and_codec()) {
    TE_ERROR("AudioEncoder: init_stream_and_codec failed");
    close();
    return false;
  }

  if (!(format_ctx_->oformat->flags & AVFMT_NOFILE)) {
    if (avio_open(&format_ctx_->pb, path_.toUtf8().constData(),
                  AVIO_FLAG_WRITE) < 0) {
      TE_ERROR("AudioEncoder: Could not open output file");
      close();
      return false;
    }
  }

  if (avformat_write_header(format_ctx_, nullptr) < 0) {
    TE_ERROR("AudioEncoder: Could not write header");
    close();
    return false;
  }

  opened_ = true;
  return true;
}

bool AudioEncoder::encode_from_buffer(const AudioBuffer &buffer) {
  if (!opened_ || !format_ctx_ || !codec_ctx_ || !stream_ || !frame_ ||
      !packet_) {
    TE_ERROR("AudioEncoder: encoder is not initialized");
    return false;
  }

  if (buffer.channels != channels_ || buffer.sample_rate != sample_rate_) {
    TE_ERROR("AudioEncoder: buffer format does not match encoder");
    return false;
  }

  const int channels = channels_;
  const int64_t total_samples =
      static_cast<int64_t>(buffer.samples.size()) / channels;

  int64_t pos = 0;
  int64_t pts = 0;

  while (pos < total_samples) {
    // Сколько сэмплов хотим в этом кадре
    const int frame_nb_samples = frame_->nb_samples;
    int64_t remaining = total_samples - pos;
    int nb = static_cast<int>(remaining < frame_nb_samples ? remaining
                                                           : frame_nb_samples);

    // Обнуляем frame и подготавливаем данные
    if (av_frame_make_writable(frame_) < 0) {
      TE_ERROR("AudioEncoder: frame not writable");
      return false;
    }

    frame_->nb_samples = nb;
    frame_->pts = pts;
    pts += nb;

    // Копируем из interleaved float в frame->data (для FLT interleaved это один
    // буфер)
    float *dst = reinterpret_cast<float *>(frame_->data[0]);
    const float *src = buffer.samples.data() + pos * channels;

    std::memcpy(dst, src, nb * channels * sizeof(float));

    pos += nb;

    // Отправляем кадр в энкодер
    if (avcodec_send_frame(codec_ctx_, frame_) < 0) {
      TE_ERROR("AudioEncoder: avcodec_send_frame failed");
      return false;
    }

    // Считываем пакеты, пока есть
    while (true) {
      int ret = avcodec_receive_packet(codec_ctx_, packet_);
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        break;
      }
      if (ret < 0) {
        TE_ERROR("AudioEncoder: avcodec_receive_packet failed");
        return false;
      }

      packet_->stream_index = stream_->index;
      av_packet_rescale_ts(packet_, codec_ctx_->time_base, stream_->time_base);

      if (av_interleaved_write_frame(format_ctx_, packet_) < 0) {
        TE_ERROR("AudioEncoder: av_interleaved_write_frame failed");
        return false;
      }

      av_packet_unref(packet_);
    }
  }

  // Флаш энкодера
  if (avcodec_send_frame(codec_ctx_, nullptr) >= 0) {
    while (true) {
      int ret = avcodec_receive_packet(codec_ctx_, packet_);
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        break;
      }
      if (ret < 0) {
        TE_ERROR("AudioEncoder: flush receive_packet failed");
        return false;
      }

      packet_->stream_index = stream_->index;
      av_packet_rescale_ts(packet_, codec_ctx_->time_base, stream_->time_base);

      if (av_interleaved_write_frame(format_ctx_, packet_) < 0) {
        TE_ERROR("AudioEncoder: av_interleaved_write_frame (flush) failed");
        return false;
      }

      av_packet_unref(packet_);
    }
  }

  return true;
}

} // namespace core
