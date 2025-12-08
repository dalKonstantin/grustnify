# Grustnify

**Grustnify** (from the Russian word *"грусть"* — *sadness*) is an audio-processing tool that transforms any audio file into a “sadified” version of itself.
The application slows down audio, lowers the pitch, adds spacious reverb, and exports the processed result in the original format (typically WAV).

The project is implemented in **C++**, using:

* **FFmpeg** for decoding and encoding
* **Qt** for UI and application framework
* **Custom DSP algorithms** for reverb and time-stretching
* **GoogleTest** for unit testing
* **spdlog** for logging

---

## Features

### ✔ Decode audio using FFmpeg

Supports common audio formats such as WAV, MP3, FLAC, AAC, etc.

### ✔ Apply DSP effects

* **Reverb** (Schroeder reverb architecture: comb filters + allpass filters)
* **Slow-down with pitch drop** (time stretching resampling)

### ✔ Encode the processed audio

Currently outputs to WAV (float PCM).

### ✔ Simple UI integration with Qt

Load → Process → Save workflow.

### ✔ Tested via GoogleTest

Audio decoding and processing are unit-tested using generated or stored WAV samples.

---

## Processing Pipeline

The audio is transformed through the following stages:

1. **Decode → AudioBuffer**
   The input file is decoded into an interleaved floating-point sample buffer.

2. **Reverb**
   A room-like reverb tail is generated using parallel comb filters followed by allpass diffusion.

3. **Slowdown (Pitch Down)**
   A simple resampling-based time-stretch algorithm stretches the audio by a factor (e.g. ×1.5).

4. **Encode & Save**
   The processed audio is written back as a WAV file, using the same sample rate and channel layout.

---

## Example Result

Input:

```
song.mp3
```

Output:

```
song_grustnified.wav
```

A slower, deeper, reverberated version designed to sound melancholic or atmospheric.

---

## Build Instructions

### Requirements

* C++20 compiler
* CMake
* FFmpeg development libraries
* Qt 6 (Widgets)
* GoogleTest (automatically fetched or system-installed)
* spdlog

### Build

```bash
cmake -B build
cmake --build build
```

### Run tests

```bash
./build/tests/run_tests
```

---

## Project Structure

```
src/
  core/
    audio_decoder.cpp/hpp
    audio_encoder.cpp/hpp
    audio_buffer.cpp/hpp
    reverb / time-stretch algorithms
  app/
    app.cpp/hpp
    main.cpp
  ui/
    main_window.cpp/hpp
  log/
    log.cpp/hpp

tests/
  test_audio_decoder.cpp
  data/
    sine_440.wav
```

---

## DSP Overview

### Reverb

Based on the classic **Schroeder reverberator**:

* 4 parallel comb filters (≈30–45 ms delays)
* 2 serial allpass filters
* Mix, room-size, and damping parameters

Produces a smooth, diffuse reverb tail.

### Slowdown + Pitch Drop

Simple resampling approach:

[
y(n) = x(n / \text{speed})
]

Result:

* longer audio
* lower pitch
* more melancholic tone

---

## Roadmap

* MP3/OGG encoding support
* Adjustable effect parameters in the UI

---

## Author

Created as an experimental project to explore FFmpeg, DSP, Qt development, and emotional audio transformation.

