#include "audio_encoder.hpp"
#include "log/log.hpp"

namespace core {

AudioEncoder::AudioEncoder() = default;

AudioEncoder::~AudioEncoder() { close(); }

void AudioEncoder::cleanup() {
  if (fifo_)
    av_audio_fifo_free(fifo_);
  if (swr_ctx_)
    swr_free(&swr_ctx_);
  if (packet_)
    av_packet_free(&packet_);
  if (frame_)
    av_frame_free(&frame_);
  if (codec_ctx_)
    avcodec_free_context(&codec_ctx_);

  if (format_ctx_) {
    if (opened_ && !(format_ctx_->oformat->flags & AVFMT_NOFILE)) {
      avio_closep(&format_ctx_->pb);
    }
    avformat_free_context(format_ctx_);
  }

  format_ctx_ = nullptr;
  codec_ctx_ = nullptr;
  swr_ctx_ = nullptr;
  fifo_ = nullptr;
  stream_ = nullptr;
  frame_ = nullptr;
  packet_ = nullptr;
  opened_ = false;
  pts_ = 0;
}

bool AudioEncoder::open(const QString &path, int sample_rate, int channels,
                        int bitrate) {
  cleanup(); // Очистка на всякий случай

  path_ = path;
  sample_rate_ = sample_rate;
  channels_ = channels;
  bitrate_ = bitrate;

  // 1. Создаём выходной AVFormatContext (угадываем формат по расширению .mp3)
  if (avformat_alloc_output_context2(&format_ctx_, nullptr, nullptr,
                                     path_.toUtf8().constData()) < 0 ||
      !format_ctx_) {
    TE_ERROR("AudioEncoder: Could not allocate output context");
    return false;
  }

  // 2. Инициализация кодека и стрима
  if (!init_stream_and_codec()) {
    TE_ERROR("AudioEncoder: init_stream_and_codec failed");
    cleanup();
    return false;
  }

  // 3. Открытие файла (если формат требует)
  if (!(format_ctx_->oformat->flags & AVFMT_NOFILE)) {
    if (avio_open(&format_ctx_->pb, path_.toUtf8().constData(),
                  AVIO_FLAG_WRITE) < 0) {
      TE_ERROR("AudioEncoder: Could not open output file");
      cleanup();
      return false;
    }
  }

  // 4. Запись заголовка
  if (avformat_write_header(format_ctx_, nullptr) < 0) {
    TE_ERROR("AudioEncoder: Could not write header");
    cleanup();
    return false;
  }

  opened_ = true;
  return true;
}

bool AudioEncoder::init_stream_and_codec() {
  // Находим MP3 кодек
  const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_MP3);
  if (!codec) {
    TE_ERROR("AudioEncoder: MP3 codec not found");
    return false;
  }

  stream_ = avformat_new_stream(format_ctx_, nullptr);
  if (!stream_)
    return false;

  codec_ctx_ = avcodec_alloc_context3(codec);
  if (!codec_ctx_)
    return false;

  // Параметры MP3
  codec_ctx_->bit_rate = bitrate_;
  codec_ctx_->sample_fmt = AV_SAMPLE_FMT_S16P; // Стандарт для LAME MP3 (Planar)
  codec_ctx_->sample_rate = sample_rate_;
  av_channel_layout_default(&codec_ctx_->ch_layout, channels_);

  // Открываем кодек
  if (avcodec_open2(codec_ctx_, codec, nullptr) < 0) {
    TE_ERROR("AudioEncoder: Could not open codec");
    return false;
  }

  avcodec_parameters_from_context(stream_->codecpar, codec_ctx_);

  // --- Инициализация SwrContext (Resampler) ---
  // Вход: Float Interleaved (из вашего AudioBuffer)
  // Выход: S16 Planar (для MP3)
  AVChannelLayout in_layout;
  av_channel_layout_default(&in_layout, channels_);

  int ret = swr_alloc_set_opts2(&swr_ctx_, &codec_ctx_->ch_layout,
                                codec_ctx_->sample_fmt, codec_ctx_->sample_rate,
                                &in_layout, AV_SAMPLE_FMT_FLT, sample_rate_, 0,
                                nullptr);
  av_channel_layout_uninit(&in_layout);

  if (ret < 0 || swr_init(swr_ctx_) < 0) {
    TE_ERROR("AudioEncoder: Could not init SwrContext");
    return false;
  }

  // --- Инициализация FIFO ---
  fifo_ = av_audio_fifo_alloc(codec_ctx_->sample_fmt, channels_, 1);
  if (!fifo_)
    return false;

  // Аллокация вспомогательных структур
  packet_ = av_packet_alloc();
  frame_ = av_frame_alloc();
  frame_->nb_samples = codec_ctx_->frame_size;
  frame_->format = codec_ctx_->sample_fmt;
  av_channel_layout_copy(&frame_->ch_layout, &codec_ctx_->ch_layout);

  if (av_frame_get_buffer(frame_, 0) < 0)
    return false;

  return true;
}

bool AudioEncoder::encode_from_buffer(const AudioBuffer &buffer) {
  if (!opened_ || !swr_ctx_ || !fifo_) {
    TE_ERROR("AudioEncoder: encoder is not initialized");
    return false;
  }

  if (buffer.channels != channels_ || buffer.sample_rate != sample_rate_) {
    TE_ERROR("AudioEncoder: buffer format does not match encoder");
    return false;
  }

  const int nb_samples = static_cast<int>(buffer.samples.size()) / channels_;
  if (nb_samples == 0)
    return true;

  // 1. Ресемплинг во временный буфер (Float -> S16P)
  uint8_t **converted_data = nullptr;
  int linesize;
  if (av_samples_alloc_array_and_samples(&converted_data, &linesize, channels_,
                                         nb_samples, codec_ctx_->sample_fmt,
                                         0) < 0) {
    TE_ERROR("AudioEncoder: could not alloc temp samples");
    return false;
  }

  const uint8_t *input_data[1] = {
      reinterpret_cast<const uint8_t *>(buffer.samples.data())};

  int ret =
      swr_convert(swr_ctx_, converted_data, nb_samples, input_data, nb_samples);
  if (ret < 0) {
    TE_ERROR("AudioEncoder: swr_convert failed");
    if (converted_data)
      av_freep(&converted_data[0]);
    free(converted_data);
    return false;
  }

  // 2. Запись в FIFO
  if (av_audio_fifo_write(fifo_, (void **)converted_data, nb_samples) <
      nb_samples) {
    TE_ERROR("AudioEncoder: fifo write failed");
    if (converted_data)
      av_freep(&converted_data[0]);
    free(converted_data);
    return false;
  }

  if (converted_data) {
    av_freep(&converted_data[0]);
    free(converted_data);
  }

  // 3. Вычитывание полных кадров из FIFO и кодирование
  while (av_audio_fifo_size(fifo_) >= codec_ctx_->frame_size) {
    if (av_frame_make_writable(frame_) < 0)
      return false;

    // Читаем ровно frame_size (1152) сэмплов
    if (av_audio_fifo_read(fifo_, (void **)frame_->data,
                           codec_ctx_->frame_size) < codec_ctx_->frame_size) {
      return false;
    }

    frame_->nb_samples = codec_ctx_->frame_size;
    frame_->pts = pts_;
    pts_ += frame_->nb_samples;

    // Отправка в кодек
    if (avcodec_send_frame(codec_ctx_, frame_) < 0) {
      TE_ERROR("AudioEncoder: avcodec_send_frame failed");
      return false;
    }

    // Получение пакетов
    while (true) {
      int ret_pkt = avcodec_receive_packet(codec_ctx_, packet_);
      if (ret_pkt == AVERROR(EAGAIN) || ret_pkt == AVERROR_EOF)
        break;
      if (ret_pkt < 0)
        return false;

      packet_->stream_index = stream_->index;
      av_packet_rescale_ts(packet_, codec_ctx_->time_base, stream_->time_base);

      if (av_interleaved_write_frame(format_ctx_, packet_) < 0) {
        TE_ERROR("AudioEncoder: write frame failed");
        return false;
      }
      av_packet_unref(packet_);
    }
  }

  return true;
}

void AudioEncoder::close() {
  if (!opened_)
    return;

  // Сначала сбрасываем остатки данных
  flush_encoder();

  // Пишем трейлер файла
  if (format_ctx_) {
    av_write_trailer(format_ctx_);
  }

  cleanup();
}

bool AudioEncoder::flush_encoder() {
  if (!fifo_ || !codec_ctx_)
    return false;

  // 1. Если в FIFO остались данные, добиваем тишиной до полного кадра
  int remaining = av_audio_fifo_size(fifo_);
  if (remaining > 0) {
    if (av_frame_make_writable(frame_) < 0)
      return false;

    av_audio_fifo_read(fifo_, (void **)frame_->data, remaining);

    // Паддинг нулями
    int padding = codec_ctx_->frame_size - remaining;
    for (int ch = 0; ch < channels_; ch++) {
      // S16P = int16_t (2 байта)
      int16_t *ptr = reinterpret_cast<int16_t *>(frame_->data[ch]);
      memset(ptr + remaining, 0, padding * sizeof(int16_t));
    }

    frame_->nb_samples = codec_ctx_->frame_size;
    frame_->pts = pts_;
    pts_ += frame_->nb_samples;

    avcodec_send_frame(codec_ctx_, frame_);

    // Забираем пакет
    while (true) {
      int ret = avcodec_receive_packet(codec_ctx_, packet_);
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        break;
      if (ret < 0)
        break;

      packet_->stream_index = stream_->index;
      av_packet_rescale_ts(packet_, codec_ctx_->time_base, stream_->time_base);
      av_interleaved_write_frame(format_ctx_, packet_);
      av_packet_unref(packet_);
    }
  }

  // 2. Финальный флаш самого кодека (передаем nullptr)
  if (avcodec_send_frame(codec_ctx_, nullptr) >= 0) {
    while (true) {
      int ret = avcodec_receive_packet(codec_ctx_, packet_);
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        break;

      packet_->stream_index = stream_->index;
      av_packet_rescale_ts(packet_, codec_ctx_->time_base, stream_->time_base);
      av_interleaved_write_frame(format_ctx_, packet_);
      av_packet_unref(packet_);
    }
  }

  return true;
}

} // namespace core
