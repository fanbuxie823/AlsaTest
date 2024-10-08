#include "AlsaWrapper.hpp"
#include <iostream>

AlsaRecorder::AlsaRecorder(uint sample_rate, uint channel) : sample_rate_(sample_rate), channels_(channel) {
}

AlsaRecorder::~AlsaRecorder() {
  if (inited_)
    DeInit();
}

//对特定的设备进行初始化
int AlsaRecorder::Init(const char* device_name) {
  snd_device_ = device_name;
  auto ret = InitSoundcard();
  if (ret == 0)
    inited_ = true;
  return ret;
}

//重置设备
int AlsaRecorder::DeInit() {
  inited_ = false;
  started_ = false;
  snd_pcm_hw_params_free(hw_params_);
  snd_pcm_close(capture_handle_);
  hw_params_ = nullptr;
  snd_config_update_free_global();
  return 0;
}

//初始化声卡
int AlsaRecorder::InitSoundcard() {
  int err = 0;

  //打开pcm句柄
  if ((err = snd_pcm_open(&capture_handle_, snd_device_, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    std::cerr << "[ALSA] cannot open audio device " << snd_device_ << "("
        << snd_strerror(err) << ", " << err << ")"
        << "\n";
    return OPEN_ERROR;
  }

  //分配hw_params结构的内存
  if ((err = snd_pcm_hw_params_malloc(&hw_params_)) < 0) {
    std::cerr << "[ALSA] cannot allocate hardware parameter structure "
        << "(" << snd_strerror(err) << ", " << err << ")"
        << "\n";
    return MALLOC_ERROR;
  }

  //Fill params with a full configuration space for a PCM.
  if ((err = snd_pcm_hw_params_any(capture_handle_, hw_params_)) < 0) {
    std::cerr << "[ALSA] cannot initialize hardware parameter structure "
        << "(" << snd_strerror(err) << ", " << err << ")"
        << "\n";
    return ANY_ERROR;
  }

  //Restrict a configuration space to contain only one access type.
  //SND_PCM_ACCESS_RW_NONINTERLEAVED。非交错访问。每个 PCM 帧只是一个声道需要的数据
  if ((err = snd_pcm_hw_params_set_access(capture_handle_, hw_params_, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    std::cerr << "[ALSA] cannot set access type " << "(" << snd_strerror(err) << ", " << err << ")"
        << "\n";
    return ACCESS_ERROR;
  }

  //设置格式Signed 16 bit Little Endian
  if ((err = snd_pcm_hw_params_set_format(capture_handle_, hw_params_, SND_PCM_FORMAT_S16_LE)) < 0) {
    std::cerr << "[ALSA] cannot set sample format "
        << "(" << snd_strerror(err) << ", " << err << ")"
        << "\n";
    return FORMAT_ERROR;
  }

  //获取相关参数
  uint channel_min = 0, channel_max = 0, samplerate_min = 0, samplerate_max = 0;
  do {
    if ((err = snd_pcm_hw_params_get_channels_min(hw_params_, &channel_min)) < 0) {
      std::cerr << "[ALSA] cannot initialize hardware parameter structure "
          << "(" << snd_strerror(err) << ", " << err << ")"
          << "\n";
      break;
    }
    if ((err = snd_pcm_hw_params_get_channels_max(hw_params_, &channel_max)) < 0) {
      std::cerr << "[ALSA] cannot initialize hardware parameter structure "
          << "(" << snd_strerror(err) << ", " << err << ")"
          << "\n";
      break;
    }
    if ((err = snd_pcm_hw_params_get_rate_min(hw_params_, &samplerate_min, nullptr)) < 0) {
      std::cerr << "[ALSA] cannot initialize hardware parameter structure "
          << "(" << snd_strerror(err) << ", " << err << ")"
          << "\n";
      break;
    }
    if ((err = snd_pcm_hw_params_get_rate_max(hw_params_, &samplerate_max, nullptr)) < 0) {
      std::cerr << "[ALSA] cannot initialize hardware parameter structure "
          << "(" << snd_strerror(err) << ", " << err << ")"
          << "\n";
      break;
    }
  } while (false);

  std::cout << "[ALSA] channel:" << channel_min << "~" << channel_max << std::endl;
  std::cout << "[ALSA] sample_rate:" << samplerate_min << "~" << samplerate_max << std::endl;

  //设置采样率
  if (snd_pcm_hw_params_test_rate(capture_handle_, hw_params_, sample_rate_, 0) != 0) {
    std::cerr << "[ALSA] samplerate " << sample_rate_ << " is not support ";
    if ((err = snd_pcm_hw_params_get_rate_min(hw_params_, &sample_rate_, nullptr)) < 0) {
      std::cerr << "\ncannot get min samplerate "
          << "(" << snd_strerror(err) << ", " << err << ")"
          << "\nstop";
      return RATE_ERROR;
    }
    std::cerr << "will set as " << sample_rate_ << std::endl;
  }
  if ((err = snd_pcm_hw_params_set_rate_near(capture_handle_, hw_params_, &sample_rate_, nullptr)) < 0) {
    std::cerr << "[ALSA] cannot set sample rate " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
    return RATE_ERROR;
  }

  //设置采样率
  if (snd_pcm_hw_params_test_channels(capture_handle_, hw_params_, channels_) != 0) {
    std::cerr << "[ALSA] channels " << channels_ << " is not support ";
    if ((err = snd_pcm_hw_params_get_channels_min(hw_params_, &channels_)) < 0) {
      std::cerr << "\ncannot get min channels "
          << "(" << snd_strerror(err) << ", " << err << ")"
          << "\nstop";
      return CHANNELS_ERROR;
    }
    std::cerr << "will set as " << channels_ << std::endl;
  }

  snd_pcm_hw_params_get_channels(hw_params_, &channels_);
  snd_pcm_hw_params_get_rate(hw_params_, &sample_rate_, nullptr);
  std::cout << "[ALSA] channel:" << channels_ << ",sample_rate:" << sample_rate_ << std::endl;
  return err;
}

//启动声卡
int AlsaRecorder::StartSoundcard() {
  int err;
  //Install one PCM hardware configuration chosen from a configuration space and snd_pcm_prepare it.
  if ((err = snd_pcm_hw_params(capture_handle_, hw_params_)) < 0) {
    std::cerr << "[ALSA] cannot set parameters " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
    return PARAMS_ERROR;
  }

  //Prepare PCM for use.
  if ((err = snd_pcm_prepare(capture_handle_)) < 0) {
    std::cerr << "[ALSA] cannot prepare audio interface for use " << "(" << snd_strerror(err) << ", " << err << ")" <<
        "\n";
    return PREPARE_ERROR;
  }

  //Start a PCM.
  if ((err = snd_pcm_start(capture_handle_)) < 0) {
    std::cerr << "[ALSA] cannot start soundcard " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
    return START_ERROR;
  }
  started_ = true;
  return 0;
}

//进行录音
int AlsaRecorder::DoRecord(char* buff, int buff_size) {
  //启动声卡
  if (!started_ && StartSoundcard() != 0) {
    return -1;
  }

  int sample_num = sample_rate_ / 50; //20ms的样本数
  int inner_buff_size = sample_num * channels_ * sizeof(short);
  char inner_buff[inner_buff_size];
  int ret = 0, filled_len = inner_buff_size;
  auto p_buff = buff;
  do {
    if ((ret = snd_pcm_readi(capture_handle_, &inner_buff, sample_num)) != sample_num) {
      std::cerr << "[ALSA] read from audio interface failed "
          << "(" << snd_strerror(ret) << ", " << ret << ")"
          << "\n";
      if (ret == -32) {
        // Broken pipe
        if ((ret = snd_pcm_prepare(capture_handle_)) < 0) {
          std::cerr << "cannot prepare audio interface for use "
              << "(" << snd_strerror(ret) << ", " << ret << ")"
              << "\n";
          return PREPARE_ERROR;
        }
      } else
        return SNDREAD_ERROR;
    }

    if (buff_size >= inner_buff_size)
      buff_size -= inner_buff_size;
    else {
      filled_len = buff_size;
      buff_size = 0;
    }
    memcpy(p_buff, inner_buff, filled_len);
    p_buff += filled_len;
  } while (buff_size > 0);
  return 0;
}