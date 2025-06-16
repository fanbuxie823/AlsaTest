#include "Recorder.hpp"
#include <iostream>
#include <cassert>

// constexpr const ulong MAX_RECORD_TIME = 5;
// constexpr const ulong SAMPLE_RATE = 44100;
// constexpr const ulong CHANNELS = 2;
// constexpr const ulong MAX_RECORD_SAMPLES = SAMPLE_RATE * MAX_RECORD_TIME;
// constexpr const ulong BUFFER_SIZE = MAX_RECORD_SAMPLES * sizeof(short) * CHANNELS;

//将双通道数据合并为单通道
int Recorder::MergeChannels(short* buff, int buff_size) {
  if (buff == nullptr) {
    return 0;
  }
  short* p_buff = buff;
  for (int i = 0; i < buff_size; i += 2) {
    p_buff[i >> 1] = (p_buff[i] + p_buff[i + 1]) >> 1;
  }
  return buff_size >> 1;
}

void Recorder::DoRecord() {
  is_recording_ = true;
  int buff_size_for_20ms = current_samplerate_ * current_channel_ * sizeof(short) * 20 / 1000;
  short audio_buff[buff_size_for_20ms]{0};
  if (!alsa_recorder_) {
    std::cerr << "[REC] alsa recorder is not init" << std::endl;
    return;
  }
  while (is_recording_) {
    int buff_size = buff_size_for_20ms;
    //从声卡读取数据
    alsa_recorder_->DoRecord(reinterpret_cast<char*>(audio_buff), buff_size * sizeof(short));
    if (!call_back_func_)
      continue;

    //如果需要合并通道
    if (need_merge_) {
      buff_size = MergeChannels(audio_buff, buff_size);
    }
    //如果需要重采样
    if (need_resample_) {
      int samples_after_resample = 0;
      auto data_after_resample = sox_resample_->Process(audio_buff, buff_size, samples_after_resample);
      call_back_func_(data_after_resample, samples_after_resample);
      continue;
    }
    call_back_func_(audio_buff, buff_size);
  }
  std::cout << "[REC] recorder quit" << std::endl;
}

Recorder::~Recorder() {
  ReleaseRecorder();
}

bool Recorder::InitRecorder(int samplerate, int channels) {
  alsa_recorder_ = std::make_unique<AlsaRecorder>(samplerate, channels);
  if (!alsa_recorder_) {
    std::cerr << "[REC] alsa create failed" << std::endl;
    return false;
  }

  //初始化
  if (alsa_recorder_->Init("default") != 0) {
    return false;
  }

  //如果是双通道的话
  current_channel_ = alsa_recorder_->GetCurrentChannels();
  current_samplerate_ = alsa_recorder_->GetCurrentSamplerate();
  if (current_samplerate_ != samplerate) {
    need_resample_ = true;
    sox_resample_ = std::make_unique<SoxResample>(current_samplerate_, samplerate, 1);
  }
  assert(current_channel_ <= 2);
  need_merge_ = current_channel_ == 2;

  return true;
}

bool Recorder::SetCallBackFunc(const RecorderCallBackFuncType& func) {
  if (func) {
    call_back_func_ = func;
    return true;
  }
  return false;
}

bool Recorder::StartRecord() {
  if (!alsa_recorder_) {
    std::cerr << "[REC] alsa recorder is not init" << std::endl;
    return false;
  }
  if (!is_recording_) {
    recorder_thread_ = std::thread(&Recorder::DoRecord, this);
  }
  return true;
}

void Recorder::StopRecord() {
  if (!is_recording_)
    return;
  is_recording_ = false;
  if (recorder_thread_.joinable()) {
    recorder_thread_.join();
  }
}

void Recorder::ReleaseRecorder() {
  if (alsa_recorder_) {
    alsa_recorder_->DeInit();
    alsa_recorder_ = nullptr;
  }
  if (sox_resample_) {
    sox_resample_ = nullptr;
  }
}